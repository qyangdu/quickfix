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

#ifndef FIX_DATADICTIONARY_H
#define FIX_DATADICTIONARY_H

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#endif

#include "Fields.h"
#include "FieldMap.h"
#include "DOMDocument.h"
#include "Exceptions.h"
#include <bitset>
#include <set>
#include <map>
#include <string.h>

#ifdef HAVE_BOOST
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#endif

#ifdef HAVE_SPARSEHASH
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>
#endif

namespace FIX
{
class FieldMap;
class Message;

/**
 * Represents a data dictionary for a version of %FIX.
 *
 * Generally loaded from an XML document.  The DataDictionary is also
 * responsible for validation beyond the basic structure of a message.
 */

class DataDictionary
{
  static const int HeaderTypeBits = 8192;
  static const int TrailerTypeBits = 8192;
  static const int DataTypeBits = 8192;
  static const int GroupTypeBits = 8192;

  enum Check { 
    FieldsOutOfOrder,
    FieldsHaveValues,
    UserDefinedFields,
    RequiredFields,
    UnknownFields,
    UnknownMsgType
  };

  static const unsigned AllChecks =
	(1 << FieldsOutOfOrder)  | (1 << FieldsHaveValues) |
	(1 << UserDefinedFields) | (1 << RequiredFields)   |
	(1 << UnknownFields)     | (1 << UnknownMsgType);

  inline void checkFor(Check c, bool value) {
    unsigned mask = 1 << c;
    if (value)
      m_checks |= mask;
    else
      m_checks &= ~mask;
  }

  inline bool isChecked(Check c) const {
    unsigned mask = 1 << c;
    return (m_checks & mask) != 0;
  }

  static inline bool isChecked(Check c, const unsigned checks) {
    unsigned mask = 1 << c;
    return (checks & mask) != 0;
  }

public:

#if defined(HAVE_BOOST)
  typedef boost::container::flat_set <
    int,
    std::less<int>,
    boost::pool_allocator<int>
  > MsgFields;

  typedef boost::unordered_map <
    String::value_type,
    MsgFields,
    ItemHash,
    String::equal_to,
    boost::fast_pool_allocator< std::pair<String::value_type, MsgFields> >
  > MsgTypeToField;

  typedef boost::container::flat_map <
    int,
    bool,
    std::less<int>,
    boost::pool_allocator< std::pair<int, bool> >
  > NonBodyFields;

  typedef std::vector<
    int,
    boost::pool_allocator<int>
  > OrderedFields;

  typedef message_order OrderedFieldsArray;

  typedef boost::unordered_set <
    String::value_type,
    ItemHash,
    String::equal_to,
    boost::fast_pool_allocator<String::value_type>
  > Values;

  typedef boost::unordered_map <
    String::value_type,
    std::pair < int, DataDictionary* >,
    ItemHash,
    std::equal_to< String::value_type >,
    boost::fast_pool_allocator< std::pair<String::value_type,
    std::pair<int, DataDictionary*> > >
  > FieldPresenceMap;

#ifdef HAVE_SPARSEHASH
  typedef google::dense_hash_set <
    String::value_type,
    ItemHash,
    String::equal_to,
    boost::pool_allocator<String::value_type>
  > MsgTypes;

  typedef google::dense_hash_set <
    int,
    boost::hash<int>,
    std::equal_to<int>,
    boost::pool_allocator<int>
  > Fields;

  typedef google::dense_hash_map <
    int,
    TYPE::Type,
    boost::hash<int>,
    std::equal_to<int>,
    boost::pool_allocator< std::pair<int, TYPE::Type> >
  > FieldTypes;

  typedef google::dense_hash_map <
    int,
    Values,
    boost::hash<int>,
    std::equal_to<int>,
    boost::pool_allocator< std::pair<int, Values> >
  > FieldToValue;

  typedef google::dense_hash_map <
    int,
    FieldPresenceMap,
    boost::hash<int>,
    std::equal_to<int>,
    boost::pool_allocator< std::pair<int, FieldPresenceMap> >
  > FieldToGroup;

#else
  typedef boost::unordered_set <
    String::value_type,
    ItemHash,
    String::equal_to,
    boost::fast_pool_allocator<String::value_type>
  > MsgTypes;

  typedef boost::unordered_set <
    int,
    boost::hash<int>,
    std::equal_to<int>,
    boost::fast_pool_allocator<int>
  > Fields;

  typedef boost::unordered_map <
    int,
    TYPE::Type,
    boost::hash<int>,
    std::equal_to<int>,
    boost::fast_pool_allocator< std::pair<int, TYPE::Type> >
  > FieldTypes;

  typedef boost::unordered_map <
    int,
    Values,
    boost::hash<int>,
    std::equal_to<int>,
    boost::fast_pool_allocator< std::pair<int, Values> >
  > FieldToValue;

  typedef boost::unordered_map <
    int,
    FieldPresenceMap,
    boost::hash<int>,
    std::equal_to<int>,
    boost::fast_pool_allocator< std::pair<int, FieldPresenceMap> >
  > FieldToGroup;

#endif
  typedef boost::unordered_map <
    int,
    std::string,
    boost::hash<int>,
    std::equal_to<int>,
    boost::fast_pool_allocator< std::pair<int, std::string> >
  > FieldToName;

  typedef boost::unordered_map <
    std::string,
    int,
    ItemHash,
    String::equal_to,
    boost::fast_pool_allocator< std::pair<std::string, int> >
  > NameToField;

  typedef boost::unordered_map <
    std::pair < int, std::string >,
    std::string,
    ItemHash,
    std::equal_to<std::pair< int, std::string> >,
    boost::fast_pool_allocator<
      std::pair<std::pair<int, std::string>,
      std::string>
    >
  > ValueToName;

#else /* !HAVE_BOOST */
  typedef std::set < int > MsgFields;
  typedef std::map < String::value_type, MsgFields > MsgTypeToField;
  typedef std::map < int, bool > NonBodyFields;
  typedef std::vector< int > OrderedFields;
  typedef message_order OrderedFieldsArray;
  typedef std::set < String::value_type > Values;
  typedef std::map < String::value_type, std::pair < int, DataDictionary* > > FieldPresenceMap;
#ifdef HAVE_SPARSEHASH
  typedef google::dense_hash_set <
    String::value_type,
    ItemHash,
    String::equal_to
  > MsgTypes;

  typedef google::dense_hash_set < int > Fields;
  typedef google::dense_hash_map < int, TYPE::Type > FieldTypes;
  typedef google::dense_hash_map < int, Values > FieldToValue;
  typedef google::dense_hash_map < int, FieldPresenceMap > FieldToGroup;
#else
  typedef std::set < String::value_type > MsgTypes;
  typedef std::set < int > Fields;
  typedef std::map < int, TYPE::Type > FieldTypes;
  typedef std::map < int, Values > FieldToValue;
  typedef std::map < int, FieldPresenceMap > FieldToGroup;
#endif
  typedef std::map < int, std::string > FieldToName;
  typedef std::map < std::string, int > NameToField;
  typedef std::map < std::pair < int, std::string > ,
  std::string  > ValueToName;
#endif /* !HAVE_BOOST */

  typedef Util::BitSet<DataTypeBits> FieldTypeDataBits;
  typedef Util::BitSet<HeaderTypeBits> FieldTypeHeaderBits;
  typedef Util::BitSet<TrailerTypeBits> FieldTypeTrailerBits;
  typedef Util::BitSet<GroupTypeBits> FieldTypeGroupBits;

  DataDictionary();
  DataDictionary( const DataDictionary& copy );
  DataDictionary( std::istream& stream ) throw( ConfigError );
  DataDictionary( const std::string& url ) throw( ConfigError );
  virtual ~DataDictionary();

  void readFromURL( const std::string& url ) throw( ConfigError );
  void readFromDocument( DOMDocumentPtr pDoc ) throw( ConfigError );
  void readFromStream( std::istream& stream ) throw( ConfigError );

  message_order const& getOrderedFields() const;

  // storage functions
  void setVersion( const std::string& beginString )
  {
    m_beginString = beginString;
    m_hasVersion = true;
  }
  const std::string& getVersion() const
  {
    return m_beginString;
  }

  void addField( int field )
  {
    m_fields.insert( field );
    m_orderedFields.push_back( field );
  }

  void addFieldName( int field, const std::string& name )
  {
    if( m_names.insert( std::make_pair(name, field) ).second == false )
      throw ConfigError( "Field named " + name + " defined multiple times" );
    m_fieldNames[field] = name;
  }

  bool getFieldName( int field, std::string& name ) const
  {
    FieldToName::const_iterator i = m_fieldNames.find( field );
    if(i == m_fieldNames.end()) return false;
    name = i->second;
    return true;
  }

  bool getFieldTag( std::string name, int& field ) const
  {
    NameToField::const_iterator i = m_names.find( name );
    if(i == m_names.end()) return false;
    field = i->second;
    return true;
  }

  void addValueName( int field, const std::string& value, const std::string& name )
  {
    m_valueNames[std::make_pair(field, value)] = name;
  }

  bool getValueName( int field, const std::string& value, std::string& name ) const
  {
    ValueToName::const_iterator i = m_valueNames.find( std::make_pair(field, value) );
    if(i == m_valueNames.end()) return false;
    name = i->second;
    return true;
  }

  bool HEAVYUSE PURE_DECL isField( int field ) const
  {
    return m_fields.find( field ) != m_fields.end();
  }

  void addMsgType( const std::string& msgType )
  {
    m_messages.insert( msgType );
  }

  bool isMsgType( const String::value_type& msgType ) const
  {
    return m_messages.find( msgType ) != m_messages.end();
  }

  void addMsgField( const std::string& msgType, int field )
  {
    m_messageFields[ msgType ].insert( field );
  }

  bool isMsgField( const String::value_type& msgType, int field ) const
  {
    MsgTypeToField::const_iterator i = m_messageFields.find( msgType );
    if ( LIKELY(i != m_messageFields.end()) )
      return i->second.find( field ) != i->second.end();
    return false;
  }

  void addHeaderField( int field, bool required )
  {
    m_headerFields[ field ] = required;
    if ( field < HeaderTypeBits )
	m_fieldTypeHeader.set(field);
  }

  bool isHeaderField( int field ) const
  {
    return ( LIKELY(field < HeaderTypeBits) )
      ? m_fieldTypeHeader[ field ] : (m_headerFields.find( field ) != m_headerFields.end());
  }

  void addTrailerField( int field, bool required )
  {
    m_trailerFields[ field ] = required;
    if ( field < TrailerTypeBits )
	m_fieldTypeTrailer.set(field);
  }

  bool isTrailerField( int field ) const
  {
    return ( LIKELY(field < TrailerTypeBits) )
      ? m_fieldTypeTrailer[ field ] : (m_trailerFields.find( field ) != m_trailerFields.end());
  }

  void addFieldType( int field, FIX::TYPE::Type type )
  {
    m_fieldTypes[ field ] = type;
    if (field < DataTypeBits && type == TYPE::Data)
	m_fieldTypeData.set(field);
  }

  bool getFieldType( int field, FIX::TYPE::Type& type ) const
  {
    FieldTypes::const_iterator i = m_fieldTypes.find( field );
    if ( i == m_fieldTypes.end() ) return false;
    type = i->second;
    return true;
  }

  void addRequiredField( const std::string& msgType, int field )
  {
    m_requiredFields[ msgType ].insert( field );
  }

  bool isRequiredField( const String::value_type& msgType, int field ) const
  {
    MsgTypeToField::const_iterator i = m_requiredFields.find( msgType );
    if ( i == m_requiredFields.end() ) return false;
    return i->second.find( field ) != i->second.end();
  }

  void addFieldValue( int field, const String::value_type& value )
  {
    m_fieldValues[ field ].insert( value );
  }

  bool hasFieldValue( int field ) const
  {
    FieldToValue::const_iterator i = m_fieldValues.find( field );
    return i != m_fieldValues.end();
  }

  bool isFieldValue( int field, const String::value_type& value) const 
  {
    FieldToValue::const_iterator i = m_fieldValues.find( field );
    return ( LIKELY(i != m_fieldValues.end()) ) ? isFieldValue( i, value ) : false;
  }

  void addGroup( const FieldPresenceMap::key_type& msg, int field, int delim,
                 const DataDictionary& dataDictionary )
  {
    DataDictionary * pDD = new DataDictionary( dataDictionary );
    pDD->setVersion( getVersion() );
 
    if (field < GroupTypeBits) m_fieldTypeGroup.set(field);
    FieldPresenceMap& presenceMap = m_groups[ field ];
    presenceMap[ String::CopyFunc()(msg) ] = std::make_pair( delim, pDD );
  }

  bool isGroup( const FieldPresenceMap::key_type& msg, int field ) const
  {
    if (LIKELY(field < GroupTypeBits && !m_fieldTypeGroup[field])) return false;
    FieldToGroup::const_iterator i = m_groups.find( field );
    if ( i == m_groups.end() ) return false;

    const FieldPresenceMap& presenceMap = i->second;

    FieldPresenceMap::const_iterator iter = presenceMap.find( msg );
    return ( iter != presenceMap.end() );
  }

  bool getGroup( const FieldPresenceMap::key_type& msg, int field, int& delim,
                 const DataDictionary*& pDataDictionary ) const
  {
    if (LIKELY(field < GroupTypeBits && !m_fieldTypeGroup[field])) return false;
    FieldToGroup::const_iterator i = m_groups.find( field );
    if ( i == m_groups.end() ) return false;

    const FieldPresenceMap& presenceMap = i->second;

    FieldPresenceMap::const_iterator iter = presenceMap.find( msg );
    if( iter == presenceMap.end() ) return false;

    std::pair < int, DataDictionary* > pair = iter->second;
    delim = pair.first;
    pDataDictionary = pair.second;
    return true;
  }

  bool isDataField( int field ) const
  {
    if ( LIKELY(field < DataTypeBits) )
      return m_fieldTypeData[ field ];
    FieldTypes::const_iterator i = m_fieldTypes.find( field );
    return i != m_fieldTypes.end() && i->second == TYPE::Data;
  }

  bool isMultipleValueField( int field ) const
  {
    FieldTypes::const_iterator i = m_fieldTypes.find( field );
    return i != m_fieldTypes.end() 
      && (i->second == TYPE::MultipleValueString 
          || i->second == TYPE::MultipleCharValue 
          || i->second == TYPE::MultipleStringValue );
  }

  void checkFieldsOutOfOrder( bool value )
  { checkFor(FieldsOutOfOrder, value); }
  void checkFieldsHaveValues( bool value )
  { checkFor(FieldsHaveValues, value); }
  void checkUserDefinedFields( bool value )
  { checkFor(UserDefinedFields, value); }
  void checkRequiredFields( bool value )
  { checkFor(RequiredFields, value); }
  void checkUnknownFields( bool value )
  { checkFor(UnknownFields, value); }
  void checkUnknownMsgType( bool value )
  { checkFor(UnknownMsgType, value); }

  /// Validate a message.
  static void validate( const Message& message,
                        const BeginString& beginString,
                        const MsgType& msgType,
                        const DataDictionary* const pSessionDD,
                        const DataDictionary* const pAppID )
  throw( FIX::Exception );

  void validate( const Message& message, bool bodyOnly ) const
  throw( FIX::Exception );

  void validate( const Message& message ) const
  throw( FIX::Exception )
  { validate( message, false ); }

  DataDictionary& operator=( const DataDictionary& rhs );

private:

  void init()
  {
#ifdef HAVE_SPARSEHASH
    m_fieldTypes.set_empty_key(-1);
    m_messages.set_empty_key(String::value_type());
    m_fieldValues.set_empty_key(-1);
    m_fields.set_empty_key(-1);
    m_groups.set_empty_key(-1);
#endif
  }

  /// Iterate through fields while applying checks.
  void iterate( const FieldMap& map, const MsgType& msgType ) const;

  /// Check if message type is defined in spec.
  void checkMsgType( const MsgType& msgType ) const
  {
    if ( LIKELY(isMsgType( msgType.forString( String::RvalFunc() ) )) )
      return;
    throw InvalidMessageType();
  }

  /// If we need to check for the tag in the dictionary
  bool shouldCheckTag( const FieldBase& field ) const
  {
    return ( !isChecked(UserDefinedFields) && field.getTag() >= FIELD::UserMin )
           ? false : isChecked(UnknownFields);
  }

  /// Check if field tag number is defined in spec.
  void checkValidTagNumber( const FieldBase& field ) const
  throw( InvalidTagNumber )
  {
    if( LIKELY(m_fields.find( field.getTag() ) != m_fields.end()) )
      return;
    throw InvalidTagNumber( field.getTag() );
  }

  void checkValidFormat( const FieldBase& field ) const
  throw( IncorrectDataFormat )
  {
    TYPE::Type type = TYPE::Unknown;
    getFieldType( field.getTag(), type );
    if( LIKELY(field.isValidType( type )) )
      return;
    throw IncorrectDataFormat( field.getTag(), field.getString() );
  }

  bool isFieldValue( const FieldToValue::const_iterator& i,
                     const String::value_type& v ) const
  {
    if( !isMultipleValueField( i->first ) )
      return i->second.find( v ) != i->second.end();

    // MultipleValue
    std::string::size_type startPos = 0;
    std::string::size_type endPos = 0;
    do
    {
      endPos = v.find_first_of(' ', startPos);
      String::value_type singleValue =
        v.substr( startPos, endPos - startPos );
      if( i->second.find( singleValue ) == i->second.end() )
        return false;
      startPos = endPos + 1;
    } while( endPos != std::string::npos );
    return true;
  }

  void checkValue( const FieldBase& field ) const
  throw( IncorrectTagValue )
  {
    int f = field.getTag();
    FieldToValue::const_iterator i = m_fieldValues.find( f );
    if ( LIKELY(i != m_fieldValues.end()) )
    {
      if ( LIKELY(isFieldValue( i, field.forString( String::RvalFunc() ) )) )
        return;
      throw IncorrectTagValue( f );
    }
  }

  /// Check if a field has a value.
  void checkHasValue( const FieldBase& field ) const
  throw( NoTagValue )
  {
    if ( LIKELY(!isChecked(FieldsHaveValues) ||
		         field.forString( String::SizeFunc() )) )
      return;
    throw NoTagValue( field.getTag() );
  }

  /// Check if a field is in this message type.
  void checkIsInMessage
  ( const FieldBase& field, const MsgType& msgType ) const
  throw( TagNotDefinedForMessage )
  {
    if ( LIKELY(isMsgField( msgType.forString( String::RvalFunc() ), field.getTag() )) )
      return;
    throw TagNotDefinedForMessage( field.getTag() );
  }

  /// Check if group count matches number of groups in
  void checkGroupCount
  ( const FieldBase& field, const FieldMap& fieldMap, const MsgType& msgType ) const
  throw( RepeatingGroupCountMismatch )
  {
    int fieldNum = field.getTag();
    if( isGroup(msgType.forString( String::RvalFunc() ), fieldNum) )
    {
      if( LIKELY(fieldMap.groupCount(fieldNum)
                 == IntConvertor::convert( field.forString( String::RvalFunc() ) )) )
        return;
      throw RepeatingGroupCountMismatch(fieldNum);
    }
  }

  /// Check if a message has all required fields.
  void checkHasRequired
  ( const FieldMap& header, const FieldMap& body, const FieldMap& trailer,
    const MsgType& msgType ) const
  throw( RequiredTagMissing )
  {
    NonBodyFields::const_iterator iNBF, iNBFend = m_headerFields.end();
    for( iNBF = m_headerFields.begin(); iNBF != iNBFend; ++iNBF )
    {
      if( LIKELY(iNBF->second != true || header.isSetField(iNBF->first)) )
        continue;
      throw RequiredTagMissing( iNBF->first );
    }

    iNBFend = m_trailerFields.end();
    for( iNBF = m_trailerFields.begin(); iNBF != iNBFend; ++iNBF )
    {
      if( LIKELY(iNBF->second != true || trailer.isSetField(iNBF->first)) )
        continue;
      throw RequiredTagMissing( iNBF->first );
    }

    MsgTypeToField::const_iterator iM
      = m_requiredFields.find( msgType.forString( String::RvalFunc() ) );
    if ( iM == m_requiredFields.end() ) return ;

    const MsgFields& fields = iM->second;
    MsgFields::const_iterator iF, iFend = fields.end();
    for( iF = fields.begin(); iF != iFend; ++iF )
    {
      if( LIKELY(body.isSetField(*iF)) )
        continue;
      throw RequiredTagMissing( *iF );
    }

    int delim;
    const DataDictionary* DD = 0;
    FieldMap::g_iterator groups, groups_end = body.g_end();
    for( groups = body.g_begin(); groups != groups_end; ++groups )
    {
      if( getGroup( msgType.forString( String::RvalFunc() ), groups->first, delim, DD ) )
      {
        FieldMap::g_item_const_iterator group, group_end = groups->second.end();
        for( group = groups->second.begin(); group != group_end; ++group )
          DD->checkHasRequired( **group, **group, **group, msgType );
      }
    }
  }

  /// Read XML file using MSXML.
  void readMSXMLDOM( const std::string& );
  void readMSXML( const std::string& );
  /// Read XML file using libXML.
  void readLibXml( const std::string& );

  int lookupXMLFieldNumber( DOMDocument*, DOMNode* ) const;
  int lookupXMLFieldNumber( DOMDocument*, const std::string& name ) const;
  int addXMLComponentFields( DOMDocument*, DOMNode*, const std::string& msgtype, DataDictionary&, bool );
  void addXMLGroup( DOMDocument*, DOMNode*, const std::string& msgtype, DataDictionary&, bool );
  TYPE::Type XMLTypeToType( const std::string& xmlType ) const;

  bool m_hasVersion;
  unsigned m_checks;
  std::string m_beginString;
  MsgTypeToField m_messageFields;
  MsgTypeToField m_requiredFields;
  MsgTypes m_messages;
  Fields m_fields;
  OrderedFields m_orderedFields;
  mutable OrderedFieldsArray m_orderedFieldsArray;
  NonBodyFields m_headerFields;
  FieldTypeHeaderBits m_fieldTypeHeader;
  FieldTypeTrailerBits m_fieldTypeTrailer;
  NonBodyFields m_trailerFields;
  FieldTypeDataBits m_fieldTypeData;
  FieldTypes m_fieldTypes;
  FieldToValue m_fieldValues;
  FieldToName m_fieldNames;
  NameToField m_names;
  ValueToName m_valueNames;
  FieldTypeGroupBits m_fieldTypeGroup;
  FieldToGroup m_groups;
};
}

#endif //FIX_DATADICTIONARY_H
