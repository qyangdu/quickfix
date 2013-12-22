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

#ifndef FIX_FIELDCONVERTORS_H
#define FIX_FIELDCONVERTORS_H

#include "FieldTypes.h"
#include "Exceptions.h"
#include "FixString.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>
#include <limits>

namespace FIX
{

#ifdef HAVE_BOOST
  namespace qi {
    using namespace boost::spirit::qi;
  }
#endif

/// String convertor is a no-op.
struct StringConvertor
{
  typedef const char* value_type;

  static std::size_t RequiredSize(value_type v) { return ::strlen(v); }
  static std::size_t RequiredSize(const std::string& v) { return String::size(v); }

  template <typename S> static void generate(S& result, const S& value)
  {
    result += value;
  }

  template <typename S> static void generate(S& result, const char* value, std::size_t size = 0)
  {
    result.append( value, size ? size : ::strlen(value) );
  }

  template <typename S> static bool parse(const char* str, const char* end, S& result)
  {
    result.assign(str, end - str);
    return true;
  }

  template <typename S> static bool parse(const std::string& value, S& result)
  {
    result = value;
    return true;
  }

  static const String::value_type& convert( const String::value_type& value )
  { return value; }

  static String::value_type convert( const char* p, std::size_t size )
  { return String::value_type(p, size); }

  template <typename S> static bool NOTHROW validate( const S& value )
  { return true; }
};

/// Converts integer/long to/from a string
struct IntConvertor
{
  public:

  typedef long value_type;
  typedef unsigned long unsigned_value_type;

  static const std::size_t MaxValueSize = std::numeric_limits<value_type>::digits10 + 2;
  static std::size_t RequiredSize(value_type v = 0) { return MaxValueSize; }

  class Proxy {
	value_type m_value;

#ifdef _MSC_VER
	inline int generate(char* buffer) const
    {
      char* p=buffer + MaxValueSize - 1;
      value_type value = m_value;
      unsigned_value_type v, u = value < 0 ? ~value + 1 : value;
      do { v = u / 10; *p-- = (char)('0' + (u - v * 10)); } while( (u = v) );
      *p = '-';
      return p - buffer + (value >= 0);
	}
#else
    inline int generate(char* buffer) const
    {
      char* p=buffer;
      value_type value = m_value;
      unsigned_value_type v, u = value < 0 ? ~value + 1 : value;
      do { v = u / 10; *p++ = (char)('0' + (u - v * 10)); } while( (u = v) );
      *p = '-';
      return ((p - buffer) + (value < 0));
    }
#endif

    public:
	Proxy(value_type value) : m_value(value) {}

	template <typename S> S convert_to() const
	{
	  char buf[MaxValueSize];
#ifdef _MSC_VER
      int offt = generate(buf);
	  return S(buf + offt, MaxValueSize - offt);
#else
#ifdef HAVE_BOOST
      return S(boost::rbegin(buf) + (MaxValueSize - generate(buf)), boost::rend(buf));
#else
      return S(Util::CharBuffer::reverse_iterator(buf + generate(buf)),
               Util::CharBuffer::reverse_iterator((char*)buf));
#endif
#endif
	}
	template <typename S> void append_to(S& s) const
    {
	  char buf[MaxValueSize];
#if _MSC_VER
      int offt = generate(buf);
	  s.append(buf + offt, MaxValueSize - offt);
#else
#ifdef HAVE_BOOST
      s.append(boost::rbegin(buf) + (MaxValueSize - generate(buf)), boost::rend(buf));
#else
      s.append(Util::CharBuffer::reverse_iterator(buf + generate(buf)),
               Util::CharBuffer::reverse_iterator((char*)buf));
#endif
#endif
	}
  };

  template <typename S> static void generate(S& result, long value)
  {
    Proxy(value).append_to(result);
  }

  template <typename T>
  static bool parse( const char* str, const char* end, T& result )
  {
    T x, multiplier = (*str == '-');
    str += multiplier;
    x = 0;
    multiplier = 1 - (multiplier << 1);
    bool valid = end > str;
    while( end > str )
    {
      const T c = *str++ - '0';
      if( LIKELY(c >= 0 && 9 >= c) )
        x = 10 * x + c;
      else
        return false;
    }
    result = x * multiplier;
    return valid;
  }

  template <typename S, typename T>
  static bool parse( const S& value, T& result )
  {
    const char* str = String::c_str(value);
    T x, multiplier = (*str == '-');
    str += multiplier;
    x = 0;
    multiplier = 1 - (multiplier << 1);
    do
    {
      const int c = *str - '0';
      if( LIKELY(c >= 0 && 9 >= c) )
        x = 10 * x + c;
      else
        return false;
    } while (*++str);

    result = x * multiplier;
    return true;
  }

  static std::string convert( long value )
  {
    return Proxy(value).convert_to<std::string>();
  }

  template <typename S> static S convert( long value )
  {
    return Proxy(value).convert_to<S>();
  }

  static long convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    long result = 0;
    if( parse( value, result ) )
      return result;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    const char* s = String::c_str(value);
    int negative = s[0] == '-';
    return ::strspn(s + negative, "0123456789") == (String::size(value) - negative);
  }
};

/// Converts positive integer
struct PositiveIntConvertor
{
  template <typename T>
  static bool parse( const char* str, const char* end, T& result )
  {
    T x = 0;
    do 
    {
      const T c = *str - '0';
      if( LIKELY(c >= 0 && 9 >= c) )
        x = 10 * x + c;
      else
        return false;
    } while ( end != ++str );
    result = x;
    return true;
  }

  template <typename S, typename T>
  static bool parse( const S& value, T& result )
  {
    const char* str = String::c_str(value);
    T x = 0;
    do
    {
      const T c = *str - '0';
      if( LIKELY(c >= 0 && 9 >= c) )
        x = 10 * x + c;
      else
        return false;
    } while (*++str);
    result = x;
    return true;
  }
};

/// Converts checksum to/from a string
struct CheckSumConvertor
{
  typedef long value_type;

  static const std::size_t MaxValueSize = 3;
  static std::size_t RequiredSize(value_type v = 0) { return MaxValueSize; }

  template <typename S> static void generate(S& result, long value)
  throw( FieldConvertError )
  {
    unsigned char n, v = (unsigned char)value;
    if ( (value - v) == 0 )
    { 
      n = v / 100;
      result.push_back('0' + n);
      v -= n * 100;
      n = v / 10;
      result.push_back('0' + n);
      v -= n * 10;
      result.push_back('0' + v);
      return;
    }
    throw FieldConvertError();
  }

  static void generate(char* result, unsigned char v)
  {
    unsigned char n;
    n = v / 100;
    result[0] = ('0' + n);
    v -= n * 100;
    n = v / 10;
    result[1] = ('0' + n);
    v -= n * 10;
    result[2] = ('0' + v);
  }

  static bool parse( const char* str, const char* end, long& result)
  {
    return PositiveIntConvertor::parse( str, end, result );
  }

  template <typename S> static bool parse( const S& value, long& result )
  {
    return PositiveIntConvertor::parse( value, result );
  }

  static std::string convert( long value )
  throw( FieldConvertError )
  {
    return convert<std::string>(value);
  }

  template <typename S> static S convert( long value )
  throw( FieldConvertError )
  {
    unsigned char n, v = (unsigned char)value;
    if ( (value - v) == 0 )
    {
      char buf[3];
      n = v / 100;
      buf[0] = '0' + n;
      v -= n * 100;
      n = v / 10;
      buf[1] = '0' + n;
      v -= n * 10;
      buf[2] = '0' + v;
      return S(buf, 3);
    }
    throw FieldConvertError();
  }

  static long convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    long result = 0;
    if( PositiveIntConvertor::parse( value, result ) )
      return result;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    const char* s = String::c_str(value);
    return ::strspn(s, "0123456789") == 3 && s[0] < 3;
  }
};

/// Converts double to/from a string
struct DoubleConvertor
{
  typedef double value_type;

  static const int MaxPrecision = 15;
  static const std::size_t MaxValueSize = /* std::log10((std::numeric_limits<double>::max)()) + MaxPrecision + 3 */ 326;
  static std::size_t RequiredSize(value_type v = 0) { return MaxValueSize; }

  class Proxy {
	const value_type m_value;
        const int m_padded;
        const bool m_round;

	int generate(char* buffer) const;

    public:
	Proxy(value_type value, int padded, bool rounded)
        : m_value(value), m_padded( (std::max)(0, (std::min)(padded, (int)MaxPrecision)) ),
          m_round(rounded)
        {}

        template <typename S> S convert_to() const
	{
		char buf[MaxValueSize];
#ifdef HAVE_BOOST
		return S(boost::rbegin(buf) + (MaxValueSize - generate(buf)), boost::rend(buf));
#else
		return S(Util::CharBuffer::reverse_iterator(buf + generate(buf)),
			 Util::CharBuffer::reverse_iterator((char*)buf));
#endif
	}
	template <typename S> void append_to(S& s) const
        {
		char buf[MaxValueSize];
#ifdef HAVE_BOOST
		s.append(boost::rbegin(buf) + (MaxValueSize - generate(buf)), boost::rend(buf));
#else
		s.append(Util::CharBuffer::reverse_iterator(buf + generate(buf)),
			 Util::CharBuffer::reverse_iterator((char*)buf));
#endif
	}
  };

  template <typename S> static void generate(S& result, double value, int padded = 0, bool rounded = false)
  {
    Proxy(value, padded, rounded).append_to(result);
  }

#ifdef HAVE_BOOST
  static bool parse( const char* str, const char* end, double& result)
  {
    qi::any_real_parser<double> x;
    return x.parse( str, end, qi::unused, qi::unused, result ) &&
           (end == str);
  }

  template <typename S> static bool parse( const S& value, double& result )
  {
    const char* str = String::c_str(value);
    const char* end = str + String::size(value);
    qi::any_real_parser<double> x;
    return x.parse( str, end, qi::unused, qi::unused, result ) &&
           (end == str);
  }
#else
  static bool parse( const char* str, const char* end, double& result)
  {
    const char * i = str;

    // Catch null strings
    if ( i >= end ) return false;
    // Eat leading '-' and recheck for null string
    if( *i == '-' && ++i == end ) return false;
    
    bool haveDigit = false;
  
    if( isdigit(*i) )
    {
      haveDigit = true;
      do
      {
        if ( ++i == end )
        {
          result = strtod( str, const_cast<char**>(&i) );
          return true;
        }
      }
      while( isdigit (*i) );
    }
  
    if( *i == '.' && ++i < end)
    {
      if ( isdigit(*i) )
      {
        haveDigit = true;
        do
        {
          if ( ++i == end )
            break;
        }
        while( isdigit (*i) );
      }
    }
  
    if( i < end || !haveDigit ) return false;
    result = strtod( str, const_cast<char**>(&i) );
    return true;
  }

  template <typename S> static bool parse( const S& value, double& result )
  {
    const char * i = String::c_str(value);
  
    // Catch null strings
    if( !*i ) return false;
    // Eat leading '-' and recheck for null string
    if( *i == '-' && !*++i ) return false;
  
    bool haveDigit = false;
  
    if( isdigit(*i) )
    {
      haveDigit = true;
      while( isdigit (*++i) );
    }
  
    if( *i == '.' && isdigit(*++i) )
    {
      haveDigit = true;
      while( isdigit (*++i) );
    }
  
    if( *i || !haveDigit ) return false;
    result = strtod( String::c_str(value), 0 );
    return true;
  }
#endif // HAVE_BOOST

  static std::string convert( double value, int padded = 0, bool rounded = false )
  {
    return Proxy(value, padded, rounded).convert_to<std::string>();
  }

  template <typename S> static S convert( double value, int padded = 0, bool rounded = false )
  {
    return Proxy(value, padded, rounded).convert_to<S>();
  }

  static double convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    double result = 0.0;
    if( parse( value, result ) )
      return result;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    const char* s = String::c_str(value);
    int negative = s[0] == '-';
    return ::strspn(s + negative, "0123456789.") == (String::size(value) - negative);
  }
};

/// Converts character to/from a string
struct CharConvertor
{
  typedef char value_type;

  static const std::size_t MaxValueSize = 1;
  static std::size_t RequiredSize(value_type v = 0) { return MaxValueSize; }

  template <typename S> static void generate(S& result, char value)
  {
    if (value != '\0' )
      result.push_back(value);
  }

  static bool parse( const char* str, const char* end, char& result )
  {
     if ( end - str == 1 )
     {
       result = *str;
       return true;
     }
     return false;
  }

  template <typename S> static bool parse( const S& value, char& result )
  {
    if( String::size(value) == 1 )
    {
      result = value[0];
      return true;
    }
    return false;
  }

  static std::string convert( char value )
  {
    return value ? std::string(1, value) : std::string();
  }

  template <typename S> static S convert( char value )
  {
    return value ? S(1, value) : S();
  }

  static char convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    char result;
    if( parse( value, result ) )
      return result;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    const char* s = String::c_str(value);
    return String::size(value) == 1 && s[0] > 32 && s[0] < 127; // ::isalnum(s[0]) || ::ispunct(s[0])
  }
};

/// Converts boolean to/from a string
struct BoolConvertor
{
  typedef bool value_type;

  static const std::size_t MaxValueSize = 1;
  static std::size_t RequiredSize(value_type v = false) { return MaxValueSize; }

  template <typename S> static void generate(S& result, bool value)
  {
    result.push_back( value ? 'Y' : 'N' );
  }

  static bool parse( const char* str, const char* end, bool& result )
  {
     if ( end - str == 1 )
     {
       char c = *str;
       result = (c == 'Y') && (c != 'N');
       return result || (c == 'N');
     }
     return false;
  }

  template <typename S> static bool parse( const S& value, bool& result )
  {
    if( String::size(value) == 1 )
    {
      char c = value[0];
      result = (c == 'Y') && (c != 'N');
      return result || (c == 'N');
    }
    return false;
  }

  static std::string convert( bool value )
  {
    const char ch = value ? 'Y' : 'N';
    return std::string( 1, ch );
  }

  template <typename S> static S convert( bool value )
  {
    const char ch = value ? 'Y' : 'N';
    return S( 1, ch );
  }

  static bool convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    bool result = false;
    if( parse( value, result ) )
      return result;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    const char* s = String::c_str(value);
    return String::size(value) == 1 && (s[0] == 'Y' || s[0] == 'N');
  }
};

struct UtcConvertorBase {

  union NumberRange {
    char     c[2];
    uint16_t u;
  };

  static ALIGN_DECL_DEFAULT NumberRange padded_numbers[100]; /* string representations of "00" to "99" */

  static inline bool parse_date(const unsigned char*& p, int& year, int& mon, int& mday)
  {
      unsigned char v;
      bool valid;

      v = *p++ - '0';
      valid = v < 10;
      year = v;
      v = *p++ - '0';
      valid = valid && v < 10;
      year = 10 * year + v;
      v = *p++ - '0';
      valid = valid && v < 10;
      year = 10 * year + v;
      v = *p++ - '0';
      valid = valid && v < 10;
      year = 10 * year + v;

      v = *p++ - '0';
      valid = valid && v < 10;
      mon = v;
      v = *p++ - '0';
      valid = valid && v < 10;
      mon = 10 * mon + v;
      valid = valid && ( mon >= 1 && mon <= 12 );

      v = *p++ - '0';
      valid = valid && v < 10;
      mday = v;
      v = *p++ - '0';
      valid = valid && v < 10;
      mday = 10 * mday + v;
      return valid && ( mday >= 1 && mday <= 31 );
  }

  static inline bool parse_time(const unsigned char*& p, int& hour, int& min, int& sec)
  {
      unsigned char v;
      bool valid;

      v = *p++ - '0';
      valid = v < 10;
      hour = v;
      v = *p++ - '0';
      valid = valid && v < 10;
      hour = 10 * hour + v;
      valid = valid && ( hour < 24 );

      valid = valid && ( *p++ == ':' );

      v = *p++ - '0';
      valid = valid && v < 10;
      min = v;
      v = *p++ - '0';
      valid = valid && v < 10;
      min = 10 * min + v;
      valid = valid && ( min < 60 );

      valid = valid && ( *p++ == ':' );

      v = *p++ - '0';
      valid = valid && v < 10;
      sec = v;
      v = *p++ - '0';
      valid = valid && v < 10;
      sec = 10 * sec + v;
      return valid && ( sec < 60 );
  }

  static inline bool parse_msec(const unsigned char*& p, int& millis)
  {
    unsigned char v;
    bool valid = ( *p++ == '.' );

    v = *p++ - '0';
    valid = valid && v < 10;
    millis = v;
    v = *p++ - '0';
    valid = valid && v < 10;
    millis = 10 * millis + v;
    v = *p++ - '0';
    valid = valid && v < 10;
    millis = 10 * millis + v;
    return valid;
  }
};

/// Converts a UtcTimeStamp to/from a string
struct UtcTimeStampConvertor : public UtcConvertorBase
{
  typedef const UtcTimeStamp& value_type;

  static const std::size_t MaxValueSize = 22;
  static std::size_t RequiredSize(value_type) { return MaxValueSize; }

  template <typename S> static void generate(S& result, const UtcTimeStamp& value, bool showMilliseconds = false)
  throw( FieldConvertError )
  {
    char buffer[ MaxValueSize ];
    union {
	char*     pc;
	uint16_t* pu;
    } b = { buffer };
    int year, month, day, hour, minute, second, millis;

    value.getYMD( year, month, day );
    value.getHMS( hour, minute, second, millis );

    if ( (unsigned)year < 10000 )
    {
      *b.pu++ = padded_numbers[(unsigned)year / 100].u;
      *b.pu++ = padded_numbers[(unsigned)year % 100].u;
      *b.pu++ = padded_numbers[(unsigned)month].u;
      *b.pu++ = padded_numbers[(unsigned)day].u;
      *b.pc++ = '-';
      *b.pu++ = padded_numbers[(unsigned)hour].u;
      *b.pc++ = ':';
      *b.pu++ = padded_numbers[(unsigned)minute].u;
      *b.pc++ = ':';
      *b.pu++ = padded_numbers[(unsigned)second].u;

      if (showMilliseconds)
      {
	*b.pc++ = '.';
	*b.pc++ = '0' + (unsigned)millis / 100;
        *b.pu   = padded_numbers[(unsigned)millis % 100 ].u;
        result.append(buffer, 21);
      }
      else
        result.append(buffer, 17);
      return;
    }
    throw FieldConvertError();
  }

  static bool parse( const char* str, const char* end, UtcTimeStamp& utc )
  {
    std::size_t sz = end - str;
    bool haveMilliseconds = (sz == 21);
    if ( haveMilliseconds || sz == 17)
    {
      const unsigned char* p = (const unsigned char*)str;
      int year, mon, mday, hour = 0, min = 0, sec = 0, millis = 0;
      bool valid = parse_date(p, year, mon, mday);

      valid = valid && (*p++ == '-');

      valid = valid && parse_time(p, hour, min, sec);

      if ( haveMilliseconds )
        valid = valid && parse_msec(p, millis);

      utc = UtcTimeStamp (hour, min, sec, millis,
                          mday, mon, year);
      return valid;
    }
    return false;
  }

  template <typename S> static bool parse( const S& value, UtcTimeStamp& utc )
  {
    const char* str = String::c_str(value);
    return parse(str, str + String::size(value), utc);
  }

  static std::string convert( const UtcTimeStamp& value,
                              bool showMilliseconds = false )
  throw( FieldConvertError )
  {
    std::string result;
    generate(result, value, showMilliseconds);
    return result;
  }

  template <typename S> static S convert( const UtcTimeStamp& value,
                                          bool showMilliseconds = false )
  throw( FieldConvertError )
  {
    S result;
    generate(result, value, showMilliseconds);
    return result;
  }

  static UtcTimeStamp convert( const String::value_type& value,
                               bool calculateDays = false )
  throw( FieldConvertError )
  {
    UtcTimeStamp utc;
    if (parse(value, utc))
      return utc;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    std::size_t sz = String::size(value);

    bool haveMilliseconds = (sz == 21);
    if ( haveMilliseconds || sz == 17)
    {
      const unsigned char* p = (const unsigned char*)String::c_str(value);
      int year, mon, mday, hour = 0, min = 0, sec = 0, millis = 0;
      bool valid = parse_date(p, year, mon, mday);

      valid = valid && (*p++ == '-');

      valid = valid && parse_time(p, hour, min, sec);

      return valid && (!haveMilliseconds || parse_msec(p, millis));
    }
    return false;
  }
};

/// Converts a UtcTimeOnly to/from a string
struct UtcTimeOnlyConvertor : public UtcConvertorBase
{
  typedef const UtcTimeOnly& value_type;

  static const std::size_t MaxValueSize = 13;
  static std::size_t RequiredSize(value_type) { return MaxValueSize; }

  template <typename S> static void generate(S& result, const UtcTimeOnly& value, bool showMilliseconds = false)
  {
    char buffer[ MaxValueSize ];
    union {
	char*     pc;
	uint16_t* pu;
    } b = { buffer };
    int hour, minute, second, millis;

    value.getHMS( hour, minute, second, millis );

    *b.pu++ = padded_numbers[(unsigned)hour].u;
    *b.pc++ = ':';
    *b.pu++ = padded_numbers[(unsigned)minute].u;
    *b.pc++ = ':';
    *b.pu++ = padded_numbers[(unsigned)second].u;

    if (showMilliseconds)
    {
      *b.pc++ = '.';
      *b.pc++ = '0' + (unsigned)millis / 100;
      *b.pu   = padded_numbers[(unsigned)millis % 100 ].u;
      result.append(buffer, 12);
    } else
      result.append(buffer, 8);
  }
  static bool parse( const char* str, const char* end, UtcTimeOnly& utc )
  {
    std::size_t sz = end - str;
    bool haveMilliseconds = (sz == 12);
    if ( haveMilliseconds || sz == 8)
    {
      const unsigned char* p = (const unsigned char*)str;
      int hour, min, sec, millis = 0;
      bool valid = parse_time(p, hour, min, sec);

      if ( haveMilliseconds )
        valid = valid && parse_msec(p, millis);

      utc = UtcTimeOnly( hour, min, sec, millis );
      return valid;
    }
    return false;
  }
  template <typename S> static bool parse( const S& value, UtcTimeOnly& utc )
  {
    const char* str = String::c_str(value);
    return parse(str, str + String::size(value), utc);
  }

  static std::string convert( const UtcTimeOnly& value,
                              bool showMilliseconds = false)
  throw( FieldConvertError )
  {
    std::string result;
    generate(result, value, showMilliseconds);
    return result;
  }

  template <typename S> static S convert( const UtcTimeOnly& value,
                                          bool showMilliseconds = false)
  throw( FieldConvertError )
  {
    S result;
    generate(result, value, showMilliseconds);
    return result;
  }

  static UtcTimeOnly convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    UtcTimeOnly utc;
    if (parse(value, utc))
      return utc;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    std::size_t sz = String::size(value);
    bool haveMilliseconds = (sz == 12);
    if ( haveMilliseconds || sz == 8)
    {
      const unsigned char* p = (const unsigned char*)String::c_str(value);
      int hour, min, sec, millis = 0;
      bool valid = parse_time(p, hour, min, sec);

      return valid && (!haveMilliseconds || parse_msec(p, millis));
    }
    return false;
  }
};

/// Converts a UtcDate to/from a string
struct UtcDateConvertor : public UtcConvertorBase
{
  typedef const UtcDate& value_type;

  static const std::size_t MaxValueSize = 9;
  static std::size_t RequiredSize(value_type) { return MaxValueSize; }

  template <typename S> static void generate(S& result, const UtcDate& value)
  throw( FieldConvertError )
  {
    char buffer[ MaxValueSize ];
    union {
	char*     pc;
	uint16_t* pu;
    } b = { buffer };
    int year, month, day;

    value.getYMD( year, month, day );

    if ( (unsigned)year < 10000 )
    {
      *b.pu++ = padded_numbers[(unsigned)year / 100].u;
      *b.pu++ = padded_numbers[(unsigned)year % 100].u;
      *b.pu++ = padded_numbers[(unsigned)month].u;
      *b.pu++ = padded_numbers[(unsigned)day].u;
      result.append(buffer, 8);
      return;
    }
    throw FieldConvertError();
  }
  static bool parse( const char* str, const char* end, UtcDate& utc )
  {
    std::size_t sz = end - str;
    if (sz == 8)
    {
      const unsigned char* p = (const unsigned char*)str;
      int year, mon, mday;
      bool valid = parse_date(p, year, mon, mday);
      utc = UtcDateOnly( mday, mon, year );
      return valid;
    }
    return false;
  }
  template <typename S> static bool parse( const S& value, UtcDate& utc )
  {
    const char* str = String::c_str(value);
    return parse(str, str + String::size(value), utc);
  }

  static std::string convert( const UtcDate& value )
  throw( FieldConvertError )
  {
    std::string result;
    generate(result, value);
    return result;
  }

  template <typename S> static S convert( const UtcDate& value )
  throw( FieldConvertError )
  {
    S result;
    generate(result, value);
    return result;
  }

  static UtcDate convert( const String::value_type& value )
  throw( FieldConvertError )
  {
    UtcDate utc;
    if (parse(value, utc))
      return utc;
    throw FieldConvertError();
  }

  template <typename S> static bool NOTHROW validate( const S& value )
  {
    int year, mon, mday;
    std::size_t sz = String::size(value);
    const unsigned char* p = (const unsigned char*)String::c_str(value);
    return (sz == 8) && parse_date(p, year, mon, mday );
  }
};

typedef UtcDateConvertor UtcDateOnlyConvertor;

typedef StringConvertor STRING_CONVERTOR;
typedef CharConvertor CHAR_CONVERTOR;
typedef DoubleConvertor PRICE_CONVERTOR;
typedef IntConvertor INT_CONVERTOR;
typedef DoubleConvertor AMT_CONVERTOR;
typedef DoubleConvertor QTY_CONVERTOR;
typedef StringConvertor CURRENCY_CONVERTOR;
typedef StringConvertor MULTIPLEVALUESTRING_CONVERTOR;
typedef StringConvertor MULTIPLESTRINGVALUE_CONVERTOR;
typedef StringConvertor MULTIPLECHARVALUE_CONVERTOR;
typedef StringConvertor EXCHANGE_CONVERTOR;
typedef UtcTimeStampConvertor UTCTIMESTAMP_CONVERTOR;
typedef BoolConvertor BOOLEAN_CONVERTOR;
typedef StringConvertor LOCALMKTDATE_CONVERTOR;
typedef StringConvertor DATA_CONVERTOR;
typedef DoubleConvertor FLOAT_CONVERTOR;
typedef DoubleConvertor PRICEOFFSET_CONVERTOR;
typedef StringConvertor MONTHYEAR_CONVERTOR;
typedef StringConvertor DAYOFMONTH_CONVERTOR;
typedef UtcDateConvertor UTCDATE_CONVERTOR;
typedef UtcTimeOnlyConvertor UTCTIMEONLY_CONVERTOR;
typedef IntConvertor NUMINGROUP_CONVERTOR;
typedef DoubleConvertor PERCENTAGE_CONVERTOR;
typedef IntConvertor SEQNUM_CONVERTOR;
typedef IntConvertor LENGTH_CONVERTOR;
typedef StringConvertor COUNTRY_CONVERTOR;
typedef StringConvertor TZTIMEONLY_CONVERTOR;
typedef StringConvertor TZTIMESTAMP_CONVERTOR;
typedef StringConvertor XMLDATA_CONVERTOR;
typedef StringConvertor LANGUAGE_CONVERTOR;
typedef CheckSumConvertor CHECKSUM_CONVERTOR;
}

#endif //FIX_FIELDCONVERTORS_H
