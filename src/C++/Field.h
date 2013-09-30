/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) quickfixengine.org  All rights reserved.
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef FIX_FIELD
#define FIX_FIELD

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <sstream>
#include <algorithm>
#include <numeric>
#include "FieldNumbers.h"
#include "FieldConvertors.h"
#include "FieldTypes.h"
#include "Utility.h"

namespace FIX
{

class FieldTag
{
  template <int N> struct TraitsDetail
  {
    enum
    {
      length = 1 + TraitsDetail<N/10>::length,
      checksum = (int)'0' + N%10 + TraitsDetail<N/10>::checksum
    };
  };

public:

  template <int N> struct Traits
  {
    enum
    {
      tag = N,
      length = TraitsDetail<N>::length + 1,
      checksum = TraitsDetail<N>::checksum + (int)'='
    };
  };

  FieldTag(int field) : m_tag(field)
    {}
  inline int getField() const
    { return m_tag; }
  inline char getTagLength() const
    { return Util::Tag::length(m_tag); }
  inline short getTagChecksum() const
    { return Util::Tag::checkSum(m_tag); }

private:
  const int m_tag;
};

/**
 * Base representation of all Field classes.
 *
 * This base class is the lowest common denominator of all fields.  It
 * keeps all fields in its most generic string representation with its
 * integer tag.
 */
class FieldBase
{
  friend class Message;

  enum Status {
	C_NONE 		= 0,
	C_METRICS	= 1,
	C_DATA		= 2,
	C_ALL		= 3	/* C_DATA | C_METRICS */
  };

protected:

  template <int Field>
  explicit FieldBase( FieldTag::Traits<Field> )
    : m_tag(Field)
    , m_tagChecksum(FieldTag::Traits<Field>::checksum)
    , m_tagLength(FieldTag::Traits<Field>::length)
    , m_calculated(C_NONE)
  {}

  template <int Field>
  explicit FieldBase( FieldTag::Traits<Field>, const std::string& string)
    : m_tag(Field)
    , m_tagChecksum(FieldTag::Traits<Field>::checksum)
    , m_tagLength(FieldTag::Traits<Field>::length)
    , m_calculated(C_NONE)
    , m_string(string)
  {}

public:
  explicit FieldBase( int field = 0 )
    : m_tag(field)
    , m_tagChecksum(Util::Tag::checkSum(field))
    , m_tagLength(Util::Tag::length(field))
    , m_calculated(C_NONE)
  {}

  template <typename Pack> 
  explicit FieldBase(const Pack& p, typename Pack::result_type* = NULL)
    : m_tag(p.getField()), m_tagChecksum(p.getTagChecksum())
    , m_tagLength(p.getTagLength()), m_calculated(C_NONE), m_string(p)
    {}

  FieldBase( int field, const std::string& string )
    : m_tag( field )
    , m_tagChecksum(Util::Tag::checkSum(field))
    , m_tagLength(Util::Tag::length(field))
    , m_calculated( C_NONE )
    , m_string(string)
    {}
/*
    FieldBase(const FieldBase& f)
	: m_tag(f.m_tag), m_tagChecksum(f.m_tagChecksum)
    , m_tagLength(f.m_tagLength), m_calculated(f.m_calculated), m_string(f.m_string)
	, m_data(f.m_data_), m_total(f.m_total)
    {}
*/
  virtual inline ~FieldBase()
    {}

  void setField( int field )
  {
    m_tag = field;
    m_tagChecksum = Util::Tag::checkSum(field);
    m_tagLength = Util::Tag::length(field);
    m_calculated = C_NONE;
  }

  void setString( const std::string& string )
  {
    m_string = string;
    m_calculated = C_NONE;
  }

  template <typename Pack> void setPacked(const Pack& p)
  {
    p.assign_to(m_string);
    m_calculated = C_NONE;
  }

  /// Get the fields integer tag.
  int getField() const
    { return m_tag; }

  /// Get the string representation of the fields value.
  const std::string& getString() const
    { return m_string; }

  /// Get the string representation of the Field (i.e.) 55=MSFT[SOH]
  const std::string& getFixString() const
  {
    populate_data();
    return m_data;
  }

  /// Push the string representation of the Field into an Sg buffer,
  /// which must be large enough to hold the result
  template <typename S> S& pushValue(S& sink) const
  {
    return ( m_calculated & C_DATA)
           ? sink.append(Util::String::c_str(m_data),
                         Util::String::length(m_data))
           : sink.append(m_tag, Util::String::c_str(m_string),
                                Util::String::length(m_string));
  }

  /// Push the string representation of the Field into a buffer
  std::string& pushValue(std::string& sink) const
  {
    if( m_calculated & C_DATA)
      sink.append(m_data);
    else
    {
      IntConvertor::generate(sink, m_tag);
      sink.push_back('=');
      sink.append(m_string);
      sink.push_back('\001');
    }
    return sink;
  }

  /// Get the length of the fields string representation
  int getLength() const
  {
    calculate_metrics();
    return m_length;
  }

  /// Get the total value the fields characters added together
  int getTotal() const
  {
    calculate_metrics();
    return m_total;
  }

  /// Compares fields based on thier tag numbers
  bool operator < ( const FieldBase& field ) const
    { return m_tag < field.m_tag; }

private:

  void calculate_metrics() const
  {
    if (!(m_calculated & C_METRICS))
    {
      m_length = Util::String::length(m_string) ;
      m_total = m_tagChecksum +
        Util::CharBuffer::checkSum(Util::String::data(m_string), m_length) +
        (int)'\001';
      m_length += m_tagLength + 1;
      m_calculated |= C_METRICS;
    }
  }
  void populate_data() const
  {
    if (!(m_calculated & C_DATA))
    {
      calculate_metrics();
  
      m_data.clear();
      m_data.reserve(m_length);
      IntConvertor::generate(m_data, m_tag);
      m_data.push_back('=');
      m_data.append(m_string);
      m_data.push_back('\001');
  
      m_calculated = C_ALL;
    }
  }

  int           m_tag;
  mutable short m_tagChecksum;
  mutable char  m_tagLength;
  mutable char  m_calculated;
  mutable int   m_length;
  mutable int   m_total;
  std::string   m_string;
  mutable std::string m_data;
};
/*! @} */

template<>
struct FieldTag::TraitsDetail<0>
{
  enum
  {
    length = 0,
    checksum = 0
  };
};

inline std::ostream& operator <<
( std::ostream& stream, const FieldBase& field )
{
  stream << field.getString();
  return stream;
}

/**
 * MSC doesn't support partial template specialization so we have this.
 * this is here to provide equality checking against native char arrays.
 */
class StringField : public FieldBase
{
  struct Data
  {
    typedef StringConvertor Convertor;

    const char* m_data;
    std::size_t m_length;

    explicit Data(const std::string& data)
    : m_data(Util::String::c_str(data)), m_length(Util::String::size(data))
    {}

    explicit Data(const char* data, std::size_t size)
    : m_data(data), m_length(size)
    {}

    explicit Data(const char* data)
    : m_data(data), m_length(::strlen(data))
    {}

    void assign_to(std::string& s) const
    {
      s.assign(m_data, m_length);
    }

    operator std::string () const
    { return std::string(m_data, m_length); }

  };

protected:

  template <int Tag> struct Packed : public Data 
  {
	typedef StringField result_type;

    explicit Packed(const std::string& data)
    : Data(data) {}

    explicit Packed(const char* data, std::size_t size)
    : Data(data, size) {}

    explicit Packed(const char* data)
    : Data(data, ::strlen(data)) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed () : Data(NULL, 0) {}
  };

  template <int Field>
  explicit StringField( FieldTag::Traits<Field> t )
  : FieldBase( t ) {}

  template <int Field>
  explicit StringField( FieldTag::Traits<Field> t, const std::string& data)
  : FieldBase( t, data) {}

public:

  struct Pack : public FieldTag, public Data
  {
	typedef StringField result_type;

    explicit Pack(int field, const std::string& data)
    : FieldTag(field), Data(data) {}

    explicit Pack(int field, const char* data, std::size_t size)
    : FieldTag(field), Data(data, size) {}

    explicit Pack(int field, const char* data)
    : FieldTag(field), Data(data) {}

    private:
      Pack() : FieldTag(0), Data(NULL, 0) {}
  };

  explicit StringField( int field, const std::string& data )
: FieldBase( field, data ) {}
  StringField( int field )
: FieldBase( field ) {}

  void setValue( const std::string& value )
    { setString( value ); }
  const std::string& getValue() const
    { return getString(); }
  operator const std::string&() const
    { return getString(); }

  bool operator<( const StringField& rhs ) const
    { return getString() < rhs.getString(); }
  bool operator>( const StringField& rhs ) const
    { return getString() > rhs.getString(); }
  bool operator==( const StringField& rhs ) const
    { return getString() == rhs.getString(); }
  bool operator!=( const StringField& rhs ) const
    { return getString() != rhs.getString(); }
  bool operator<=( const StringField& rhs ) const
    { return getString() <= rhs.getString(); }
  bool operator>=( const StringField& rhs ) const
    { return getString() >= rhs.getString(); }
  friend bool operator<( const StringField&, const char* );
  friend bool operator<( const char*, const StringField& );
  friend bool operator>( const StringField&, const char* );
  friend bool operator>( const char*, const StringField& );
  friend bool operator==( const StringField&, const char* );
  friend bool operator==( const char*, const StringField& );
  friend bool operator!=( const StringField&, const char* );
  friend bool operator!=( const char*, const StringField& );
  friend bool operator<=( const StringField&, const char* );
  friend bool operator<=( const char*, const StringField& );
  friend bool operator>=( const StringField&, const char* );
  friend bool operator>=( const char*, const StringField& );

  friend bool operator<( const StringField&, const std::string& );
  friend bool operator<( const std::string&, const StringField& );
  friend bool operator>( const StringField&, const std::string& );
  friend bool operator>( const std::string&, const StringField& );
  friend bool operator==( const StringField&, const std::string& );
  friend bool operator==( const std::string&, const StringField& );
  friend bool operator!=( const StringField&, const std::string& );
  friend bool operator!=( const std::string&, const StringField& );
  friend bool operator<=( const StringField&, const std::string& );
  friend bool operator<=( const std::string&, const StringField& );
  friend bool operator>=( const StringField&, const std::string& );
  friend bool operator>=( const std::string&, const StringField& );
};

inline bool operator<( const StringField& lhs, const char* rhs )
  { return lhs.getValue() < rhs; }
inline bool operator<( const char* lhs, const StringField& rhs )
  { return lhs < rhs.getValue(); }
inline bool operator>( const StringField& lhs, const char* rhs )
  { return lhs.getValue() > rhs; }
inline bool operator>( const char* lhs, const StringField& rhs )
  { return lhs > rhs.getValue(); }
inline bool operator==( const StringField& lhs, const char* rhs )
  { return lhs.getValue() == rhs; }
inline bool operator==( const char* lhs, const StringField& rhs )
  { return lhs == rhs.getValue(); }
inline bool operator!=( const StringField& lhs, const char* rhs )
  { return lhs.getValue() != rhs; }
inline bool operator!=( const char* lhs, const StringField& rhs )
  { return lhs != rhs.getValue(); }
inline bool operator<=( const StringField& lhs, const char* rhs )
  { return lhs.getValue() <= rhs; }
inline bool operator<=( const char* lhs, const StringField& rhs )
  { return lhs <= rhs.getValue(); }
inline bool operator>=( const StringField& lhs, const char* rhs )
  { return lhs.getValue() >= rhs; }
inline bool operator>=( const char* lhs, const StringField& rhs )
  { return lhs >= rhs.getValue(); }

inline bool operator<( const StringField& lhs, const std::string& rhs )
  { return lhs.getValue() < rhs; }
inline bool operator<( const std::string& lhs, const StringField& rhs )
  { return lhs < rhs.getValue(); }
inline bool operator>( const StringField& lhs, const std::string& rhs )
  { return lhs.getValue() > rhs; }
inline bool operator>( const std::string& lhs, const StringField& rhs )
  { return lhs > rhs.getValue(); }
inline bool operator==( const StringField& lhs, const std::string& rhs )
  { return lhs.getValue() == rhs; }
inline bool operator==( const std::string& lhs, const StringField& rhs )
  { return lhs == rhs.getValue(); }
inline bool operator!=( const StringField& lhs, const std::string& rhs )
  { return lhs.getValue() != rhs; }
inline bool operator!=( const std::string& lhs, const StringField& rhs )
  { return lhs != rhs.getValue(); }
inline bool operator<=( const StringField& lhs, const std::string& rhs )
  { return lhs.getValue() <= rhs; }
inline bool operator<=( const std::string& lhs, const StringField& rhs )
  { return lhs <= rhs.getValue(); }
inline bool operator>=( const StringField& lhs, const std::string& rhs )
  { return lhs.getValue() >= rhs; }
inline bool operator>=( const std::string& lhs, const StringField& rhs )
  { return lhs >= rhs.getValue(); }

/// Field that contains a character value
class CharField : public FieldBase
{
  struct Data {

    typedef CharConvertor Convertor;

    char m_data;

    explicit Data(char data) : m_data(data) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data);
    }

    operator std::string() const
    { return Convertor::convert(m_data); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef CharField result_type;

    explicit Packed(char data )
    : Data( data ) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(0) {}
  };

  template <int Field>
  explicit CharField( FieldTag::Traits<Field> t )
  : FieldBase( t ) {}

  template <int Field>
  explicit CharField( FieldTag::Traits<Field>, char data)
  : FieldBase( Packed<Field>(data) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef CharField result_type;

    explicit Pack(int field, char data )
    : FieldTag(field), Data( data ) {}

    private:
      Pack() : FieldTag(0), Data(0) {}
  };

  explicit CharField( int field, char data )
: FieldBase( Pack(field, data) ) {}
  CharField( int field )
: FieldBase( field ) {}

  void setValue( char value )
    { setPacked( Packed<0>(value) ); }
  char getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator char() const
    { return getValue(); }
};

/// Field that contains a double value
class DoubleField : public FieldBase
{
  struct Data {

    typedef DoubleConvertor Convertor;

    double m_data;
    int m_padded;
    bool m_round;

    explicit Data(double data, int padding, bool rounded)
    : m_data(data), m_padded(padding), m_round(rounded)
    {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data, m_padded, m_round);
    }

    operator std::string () const
    { return Convertor::convert(m_data, m_padded, m_round); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef DoubleField result_type;

    explicit Packed( double data, int padding = 0, bool rounded = false )
    : Data(data, padding, rounded) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(0, 0, 0) {}
  };

  template <int Field>
  explicit DoubleField( FieldTag::Traits<Field> t )
  : FieldBase( t ) {}

  template <int Field>
  explicit DoubleField( FieldTag::Traits<Field>,
                        double data, int padding = 0, bool rounded = false)
  : FieldBase( Packed<Field>(data, padding, rounded) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef DoubleField result_type;

    explicit Pack( int field,
                   double data, int padding = 0, bool rounded = false )
    : FieldTag(field), Data(data, padding, rounded) {}

    private:
      Pack() : FieldTag(0), Data(0, 0, 0) {}
  };

  explicit DoubleField( int field,
                        double data, int padding = 0, bool rounded = false )
: FieldBase( Pack(field, data, padding, rounded) ) {}
  DoubleField( int field )
: FieldBase( field ) {}

  void setValue( double value, int padding = 0 )
    { setPacked( Packed<0>(value, padding, false ) ); }
  void setValue( double value, int padding, bool rounded )
    { setPacked( Packed<0>(value, padding, rounded ) ); }
  double getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator double() const
    { return getValue(); }
};

/// Field that contains an integer value
class IntField : public FieldBase
{
  struct Data
  {
    typedef IntConvertor Convertor;
    
    int m_data;

    Data(int data) : m_data(data) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data);
    }

    operator std::string () const
    { return Convertor::convert(m_data); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
	typedef IntField result_type;

    explicit Packed( int data )
    : Data( data ) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(0) {}
  };

  template <int Field>
  explicit IntField( FieldTag::Traits<Field> t )
  : FieldBase( t ) {}

  template <int Field>
  explicit IntField( FieldTag::Traits<Field>, int value)
  : FieldBase( Packed<Field>(value) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
	typedef IntField result_type;

    explicit Pack( int field, int data )
    : FieldTag(field), Data( data ) {}

    private:
      Pack() : FieldTag(0), Data(0) {}
  };

  explicit IntField( int field, int data )
: FieldBase( Pack( field, data ) ) {}
  IntField( int field )
: FieldBase( field ) {}

  void setValue( int value )
    { setPacked( Packed<0>( value ) ); }
  int getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator const int() const
    { return getValue(); }

};

/// Field that contains a boolean value
class BoolField : public FieldBase
{
  struct Data
  {
    typedef BoolConvertor Convertor;

    bool m_data;

    Data(bool data) : m_data(data) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data);
    }

    operator std::string () const
    { return Convertor::convert(m_data); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef BoolField result_type;

    explicit Packed( bool data )
    : Data( data ) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(false) {}
  };

  template <int Field>
  explicit BoolField( FieldTag::Traits<Field> t )
  : FieldBase( t ) {}

  template <int Field>
  explicit BoolField( FieldTag::Traits<Field>, bool data)
  : FieldBase( Packed<Field>(data) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef BoolField result_type;

    explicit Pack( int field, bool data )
    : FieldTag(field), Data( data ) {}

    private:
      Pack() : FieldTag(0), Data(false) {}
  };

  explicit BoolField( int field, bool data )
: FieldBase( Pack(field, data ) ) {}
  BoolField( int field )
: FieldBase( field ) {}

  void setValue( bool value )
    { setPacked( Packed<0>( value ) ); }
  bool getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator bool() const
    { return getValue(); }
};

/// Field that contains a UTC time stamp value
class UtcTimeStampField : public FieldBase
{
  struct Data
  {
    typedef UtcTimeStampConvertor Convertor;

    UtcTimeStamp m_data;
    bool m_msec;

    Data(const UtcTimeStamp& data, bool showMilliseconds)
    : m_data(data), m_msec(showMilliseconds) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data, m_msec);
    }

    operator std::string () const
    { return Convertor::convert(m_data, m_msec); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef UtcTimeStampField result_type;

	explicit Packed( const UtcTimeStamp& data, bool showMilliseconds = false )
    : Data( data, showMilliseconds) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(UtcTimeStamp(0, 0, 0, 0, 0, 0), false) {}
  };

  template <int Field>
  explicit UtcTimeStampField( FieldTag::Traits<Field>, bool showMilliseconds )
  : FieldBase( Packed<Field>( UtcTimeStamp(), showMilliseconds) ) {}

  template <int Field>
  explicit UtcTimeStampField( FieldTag::Traits<Field>,
                              const UtcTimeStamp& data, bool showMilliseconds )
  : FieldBase( Packed<Field>(data, showMilliseconds) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef UtcTimeStampField result_type;

    explicit Pack( int field,
               const UtcTimeStamp& data, bool showMilliseconds = false )
    : FieldTag(field), Data(data, showMilliseconds) {}

    private:
      Pack() : FieldTag(0), Data(UtcTimeStamp(0, 0, 0, 0, 0, 0), false) {}
  };

  explicit UtcTimeStampField( int field,
               const UtcTimeStamp& data, bool showMilliseconds = false )
: FieldBase( Pack( field, data, showMilliseconds ) ) {}
  UtcTimeStampField( int field, bool showMilliseconds = false )
: FieldBase( Pack( field, UtcTimeStamp(), showMilliseconds ) ) {}

  void setValue( const UtcTimeStamp& value )
    { setPacked( Packed<0>( value ) ); }
  UtcTimeStamp getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator UtcTimeStamp() const
    { return getValue(); }

  bool operator<( const UtcTimeStampField& rhs ) const
    { return getValue() < rhs.getValue(); }
  bool operator==( const UtcTimeStampField& rhs ) const
    { return getValue() == rhs.getValue(); }
  bool operator!=( const UtcTimeStampField& rhs ) const
    { return getValue() != rhs.getValue(); }
};

/// Field that contains a UTC date value
class UtcDateField : public FieldBase
{
  struct Data
  {
    typedef UtcDateConvertor Convertor;

    UtcDate m_data;

    Data(const UtcDate& data)
    : m_data(data) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data);
    }

    operator std::string () const
    { return Convertor::convert(m_data); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef UtcDateField result_type;

    explicit Packed( const UtcDate& data )
    : Data( data ) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(UtcDate(0, 0, 0)) {}
  };

  template <int Field>
  explicit UtcDateField( FieldTag::Traits<Field> )
  : FieldBase( Packed<Field>( UtcDate() ) ) {}

  template <int Field>
  explicit UtcDateField( FieldTag::Traits<Field>, const UtcDate& data )
  : FieldBase( Packed<Field>(data) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef UtcDateField result_type;

    explicit Pack( int field, const UtcDate& data )
    : FieldTag(field), Data( data ) {}

    private:
      Pack() : FieldTag(0), Data(UtcDate(0, 0, 0)) {}
  };

  explicit UtcDateField( int field, const UtcDate& data )
: FieldBase( Pack( field, data ) ) {}
  UtcDateField( int field )
: FieldBase( Pack( field, UtcDate() ) ) {}

  void setValue( const UtcDate& value )
    { setPacked( Packed<0>( value ) ); }
  UtcDate getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator UtcDate() const
    { return getValue(); }

  bool operator<( const UtcDateField& rhs ) const
    { return getValue() < rhs.getValue(); }
  bool operator==( const UtcDateField& rhs ) const
    { return getValue() == rhs.getValue(); }
  bool operator!=( const UtcDateField& rhs ) const
    { return getValue() != rhs.getValue(); }
};

/// Field that contains a UTC time value
class UtcTimeOnlyField : public FieldBase
{
  struct Data
  {
    typedef UtcTimeOnlyConvertor Convertor;

    UtcTimeOnly m_data;
    bool m_msec;

    Data(const UtcTimeOnly& data, bool showMilliseconds)
    : m_data(data), m_msec(showMilliseconds) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data, m_msec);
    }

    operator std::string () const
    { return Convertor::convert(m_data, m_msec); }
  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef UtcTimeOnlyField result_type;

    explicit Packed( const UtcTimeOnly& data, bool showMilliseconds = false )
    : Data( data, showMilliseconds) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(UtcTimeOnly(0, 0, 0, 0), false) {}
  };

  template <int Field>
  explicit UtcTimeOnlyField( FieldTag::Traits<Field>, bool showMilliseconds )
  : FieldBase( Packed<Field>( UtcTimeOnly(), showMilliseconds ) ) {}

  template <int Field>
  explicit UtcTimeOnlyField( FieldTag::Traits<Field>,
                             const UtcTimeOnly& data, bool showMilliseconds )
  : FieldBase( Packed<Field>( data, showMilliseconds ) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef UtcTimeOnlyField result_type;

    explicit Pack( int field,
                     const UtcTimeOnly& data, bool showMilliseconds = false )
    : FieldTag(field), Data( data, showMilliseconds) {}

    private:
      Pack() : FieldTag(0), Data(UtcTimeOnly(0, 0, 0, 0), false) {}
  };

  explicit UtcTimeOnlyField( int field,
                     const UtcTimeOnly& data, bool showMilliseconds = false )
: FieldBase( Pack( field, data, showMilliseconds ) ) {}
  UtcTimeOnlyField( int field, bool showMilliseconds = false )
: FieldBase( Pack( field, UtcTimeOnly(), showMilliseconds ) ) {}

  void setValue( const UtcTimeOnly& value )
    { setPacked( Packed<0>( value ) ); }
  UtcTimeOnly getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator UtcTimeOnly() const
    { return getValue(); }

  bool operator<( const UtcTimeOnlyField& rhs ) const
    { return getValue() < rhs.getValue(); }
  bool operator==( const UtcTimeOnlyField& rhs ) const
    { return getValue() == rhs.getValue(); }
  bool operator!=( const UtcTimeOnlyField& rhs ) const
    { return getValue() != rhs.getValue(); }
};

/// Field that contains a checksum value
class CheckSumField : public FieldBase
{
  struct Data
  {
    typedef CheckSumConvertor Convertor;

    int m_data;

    Data(int data)
    : m_data(data) {}

    void assign_to(std::string& s) const
    {
      s.clear();
      Convertor::generate(s, m_data);
    }

    operator std::string () const
    { return Convertor::convert(m_data); }

  };

protected:

  template <int Tag> struct Packed : public Data
  {
    typedef CheckSumField result_type;

	explicit Packed( int data )
    : Data( data ) {}

    static inline int getField() { return Tag; }
    static inline char getTagLength()
    { return FieldTag::Traits<Tag>::length; }
    static inline short getTagChecksum()
    { return FieldTag::Traits<Tag>::checksum; }

    private:
      Packed() : Data(0) {}
  };

  template <int Field>
  explicit CheckSumField( FieldTag::Traits<Field> t )
  : FieldBase( t ) {}

  template <int Field>
  explicit CheckSumField( FieldTag::Traits<Field>, int data)
  : FieldBase( Packed<Field>(data) ) {}

public:

  struct Pack : public FieldTag, public Data
  {
    typedef CheckSumField result_type;

    explicit Pack( int field, int data )
    : FieldTag(field), Data( data ) {}

    private:
      Pack() : FieldTag(0), Data(0) {}
  };

  explicit CheckSumField( int field, int data )
: FieldBase( Pack( field, data ) ) {}
  CheckSumField( int field )
: FieldBase( field ) {}

  void setValue( int value )
    { setPacked( Packed<0>( value ) ); }
  int getValue() const throw ( IncorrectDataFormat )
    { try
      { return Data::Convertor::convert( getString() ); }
      catch( FieldConvertError& )
      { throw IncorrectDataFormat( getField(), getString() ); }
    }
  operator const int() const
    { return getValue(); }
};

typedef DoubleField PriceField;
typedef DoubleField AmtField;
typedef DoubleField QtyField;
typedef StringField CurrencyField;
typedef StringField MultipleValueStringField;
typedef StringField MultipleStringValueField;
typedef StringField MultipleCharValueField;
typedef StringField ExchangeField;
typedef StringField LocalMktDateField;
typedef StringField DataField;
typedef DoubleField FloatField;
typedef DoubleField PriceOffsetField;
typedef StringField MonthField;
typedef StringField MonthYearField;
typedef StringField DayOfMonthField;
typedef UtcDateField UtcDateOnlyField;
typedef IntField LengthField;
typedef IntField NumInGroupField;
typedef IntField SeqNumField;
typedef DoubleField PercentageField;
typedef StringField CountryField;
typedef StringField TzTimeOnlyField;
typedef StringField TzTimeStampField;
}

#define DEFINE_FIELD_CLASS_NUM( NAME, TOK, TYPE, NUM ) \
class NAME : public TOK##Field { public: \
typedef TOK##Field::Packed<NUM> Pack; \
NAME() : TOK##Field( FieldTag::Traits<NUM>()) {} \
NAME(const TYPE& value) : TOK##Field( FieldTag::Traits<NUM>(), value) {} \
}

#define DEFINE_FIELD_CLASS( NAME, TOK, TYPE ) \
DEFINE_FIELD_CLASS_NUM(NAME, TOK, TYPE, FIELD::NAME)

#define DEFINE_DEPRECATED_FIELD_CLASS( NAME, TOK, TYPE ) \
DEFINE_FIELD_CLASS_NUM(NAME, TOK, TYPE, DEPRECATED_FIELD::NAME)

#define DEFINE_FIELD_TIMECLASS_NUM( NAME, TOK, TYPE, NUM ) \
class NAME : public TOK##Field { public: \
typedef TOK##Field::Packed<NUM> Pack; \
NAME() : TOK##Field( FieldTag::Traits<NUM>(), false) {} \
NAME(bool showMilliseconds) : TOK##Field( FieldTag::Traits<NUM>(), showMilliseconds) {} \
NAME(const TYPE& value) : TOK##Field( FieldTag::Traits<NUM>(), value, false) {} \
NAME(const TYPE& value, bool showMilliseconds) : TOK##Field( FieldTag::Traits<NUM>(), value, showMilliseconds) {} \
}

#define DEFINE_FIELD_TIMECLASS( NAME, TOK, TYPE ) \
DEFINE_FIELD_TIMECLASS_NUM(NAME, TOK, TYPE, FIELD::NAME)

#define DEFINE_DEPRECATED_FIELD_TIMECLASS( NAME, TOK, TYPE ) \
DEFINE_FIELD_TIMECLASS_NUM(NAME, TOK, TYPE, DEPRECATED_FIELD::NAME)

#define DEFINE_CHECKSUM( NAME ) \
  DEFINE_FIELD_CLASS(NAME, CheckSum, FIX::INT)
#define DEFINE_STRING( NAME ) \
  DEFINE_FIELD_CLASS(NAME, String, FIX::STRING)
#define DEFINE_CHAR( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Char, FIX::CHAR)
#define DEFINE_PRICE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Price, FIX::PRICE)
#define DEFINE_INT( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Int, FIX::INT)
#define DEFINE_AMT( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Amt, FIX::AMT)
#define DEFINE_QTY( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Qty, FIX::QTY)
#define DEFINE_CURRENCY( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Currency, FIX::CURRENCY)
#define DEFINE_MULTIPLEVALUESTRING( NAME ) \
  DEFINE_FIELD_CLASS(NAME, MultipleValueString, FIX::MULTIPLEVALUESTRING)
#define DEFINE_MULTIPLESTRINGVALUE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, MultipleStringValue, FIX::MULTIPLESTRINGVALUE)
#define DEFINE_MULTIPLECHARVALUE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, MultipleCharValue, FIX::MULTIPLECHARVALUE)
#define DEFINE_EXCHANGE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Exchange, FIX::EXCHANGE)
#define DEFINE_UTCTIMESTAMP( NAME ) \
  DEFINE_FIELD_TIMECLASS(NAME, UtcTimeStamp, FIX::UTCTIMESTAMP)
#define DEFINE_BOOLEAN( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Bool, FIX::BOOLEAN)
#define DEFINE_LOCALMKTDATE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, String, FIX::LOCALMKTDATE)
#define DEFINE_DATA( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Data, FIX::DATA)
#define DEFINE_FLOAT( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Float, FIX::FLOAT)
#define DEFINE_PRICEOFFSET( NAME ) \
  DEFINE_FIELD_CLASS(NAME, PriceOffset, FIX::PRICEOFFSET)
#define DEFINE_MONTHYEAR( NAME ) \
  DEFINE_FIELD_CLASS(NAME, MonthYear, FIX::MONTHYEAR)
#define DEFINE_DAYOFMONTH( NAME ) \
  DEFINE_FIELD_CLASS(NAME, DayOfMonth, FIX::DAYOFMONTH)
#define DEFINE_UTCDATE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, UtcDate, FIX::UTCDATE)
#define DEFINE_UTCDATEONLY( NAME ) \
  DEFINE_FIELD_CLASS(NAME, UtcDateOnly, FIX::UTCDATEONLY)
#define DEFINE_UTCTIMEONLY( NAME ) \
  DEFINE_FIELD_TIMECLASS(NAME, UtcTimeOnly, FIX::UTCTIMEONLY)
#define DEFINE_NUMINGROUP( NAME ) \
  DEFINE_FIELD_CLASS(NAME, NumInGroup, FIX::NUMINGROUP)
#define DEFINE_SEQNUM( NAME ) \
  DEFINE_FIELD_CLASS(NAME, SeqNum, FIX::SEQNUM)
#define DEFINE_LENGTH( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Length, FIX::LENGTH)
#define DEFINE_PERCENTAGE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Percentage, FIX::PERCENTAGE)
#define DEFINE_COUNTRY( NAME ) \
  DEFINE_FIELD_CLASS(NAME, Country, FIX::COUNTRY)
#define DEFINE_TZTIMEONLY( NAME ) \
  DEFINE_FIELD_CLASS(NAME, String, FIX::TZTIMEONLY)
#define DEFINE_TZTIMESTAMP( NAME ) \
  DEFINE_FIELD_CLASS(NAME, String, FIX::TZTIMESTAMP)
#define DEFINE_XMLDATA( NAME ) \
  DEFINE_FIELD_CLASS(NAME, String, FIX::XMLDATA)
#define DEFINE_LANGUAGE( NAME ) \
  DEFINE_FIELD_CLASS(NAME, String, FIX::LANGUAGE)

#define USER_DEFINE_STRING( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::STRING, NUM)
#define USER_DEFINE_CHAR( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Char, FIX::CHAR, NUM)
#define USER_DEFINE_PRICE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Price, FIX::PRICE, NUM)
#define USER_DEFINE_INT( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Int, FIX::INT, NUM)
#define USER_DEFINE_AMT( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Amt, FIX::AMT, NUM)
#define USER_DEFINE_QTY( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Qty, FIX::QTY, NUM)
#define USER_DEFINE_CURRENCY( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Currency, FIX::CURRENCY, NUM)
#define USER_DEFINE_MULTIPLEVALUESTRING( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, MultipleValueString, FIX::MULTIPLEVALUESTRING, NUM)
#define USER_DEFINE_MULTIPLESTRINGVALUE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, MultipleStringValue, FIX::MULTIPLESTRINGVALUE, NUM)
#define USER_DEFINE_MULTIPLECHARVALUE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, MultipleCharValue, FIX::MULTIPLECHARVALUE, NUM)
#define USER_DEFINE_EXCHANGE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Exchange, FIX::EXCHANGE, NUM)
#define USER_DEFINE_UTCTIMESTAMP( NAME, NUM ) \
  DEFINE_FIELD_TIMECLASS_NUM(NAME, UtcTimeStamp, FIX::UTCTIMESTAMP, NUM)
#define USER_DEFINE_BOOLEAN( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Bool, FIX::BOOLEAN, NUM)
#define USER_DEFINE_LOCALMKTDATE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::STRING, NUM)
#define USER_DEFINE_DATA( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Data, FIX::DATA, NUM)
#define USER_DEFINE_FLOAT( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Float, FIX::FLOAT, NUM)
#define USER_DEFINE_PRICEOFFSET( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, PriceOffset, FIX::PRICEOFFSET, NUM)
#define USER_DEFINE_MONTHYEAR( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, MonthYear, FIX::MONTHYEAR, NUM)
#define USER_DEFINE_DAYOFMONTH( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, DayOfMonth, FIX::DAYOFMONTH, NUM)
#define USER_DEFINE_UTCDATE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, UtcDate, FIX::UTCDATE, NUM)
#define USER_DEFINE_UTCDATEONLY( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, UtcDateOnly, FIX::UTCDATEONLY, NUM)
#define USER_DEFINE_UTCTIMEONLY( NAME, NUM ) \
  DEFINE_FIELD_TIMECLASS_NUM(NAME, UtcTimeOnly, FIX::UTCTIMEONLY, NUM)
#define USER_DEFINE_NUMINGROUP( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, NumInGroup, FIX::NUMINGROUP, NUM)
#define USER_DEFINE_SEQNUM( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, SeqNum, FIX::SEQNUM, NUM)
#define USER_DEFINE_LENGTH( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Length, FIX::LENGTH, NUM)
#define USER_DEFINE_PERCENTAGE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Percentage, FIX::PERCENTAGE, NUM)
#define USER_DEFINE_COUNTRY( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, Country, FIX::COUNTRY, NUM)
#define USER_DEFINE_TZTIMEONLY( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::TZTIMEONLY, NUM)
#define USER_DEFINE_TZTIMESTAMP( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::TZTIMESTAMP, NUM)
#define USER_DEFINE_XMLDATA( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::XMLDATA, NUM)
#define USER_DEFINE_LANGUAGE( NAME, NUM ) \
  DEFINE_FIELD_CLASS_NUM(NAME, String, FIX::LANGUAGE, NUM)

#endif

