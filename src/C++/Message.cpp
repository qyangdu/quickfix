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

#ifdef _MSC_VER
#include "stdafx.h"
#else
#include "config.h"
#endif

#include "Message.h"
#include "Utility.h"
#include "Values.h"
#include <iomanip>

namespace FIX
{
const char* Message::FieldReader::ErrDelimiter = "Equal sign not found in field";
const char* Message::FieldReader::ErrTag = "Malformed field tag";
const char* Message::FieldReader::ErrSOH = "SOH not found at end of field";

std::auto_ptr<DataDictionary> Message::s_dataDictionary;

int Message::FieldCounter::countGroups(FieldMap::g_const_iterator git,
                               const FieldMap::g_const_iterator& gend)
{
  int result;
  for( result = 0 ; git != gend; ++git )
  {
    FieldMap::g_item_const_iterator end = git->second.end();
    for ( FieldMap::g_item_const_iterator it = git->second.begin();
          it != end; ++it )
      result += countBody(**it);
  }
  return result;
}

Message::Message()
: FieldMap( FieldMap::create_allocator() ),
  m_header( get_allocator(), message_order( message_order::header ) ),
  m_trailer( get_allocator(), message_order( message_order::trailer ) ),
  m_status( 0 ) {}

Message::Message( bool hintSerializeOnce )
: FieldMap( FieldMap::create_allocator() ),
  m_header( get_allocator(), message_order( message_order::header ) ),
  m_trailer( get_allocator(), message_order( message_order::trailer ) ),
  m_status( createStatus(hint_no_reuse, hintSerializeOnce) ) {}

Message::Message( const std::string& string, bool validate )
throw( InvalidMessage )
: FieldMap( FieldMap::create_allocator() ),
  m_header( get_allocator(), message_order( message_order::header ) ),
  m_trailer( get_allocator(), message_order( message_order::trailer ) ),
  m_status( 0 )
{
  setString( string, validate );
}

Message::Message( const std::string& string,
                  const DataDictionary& dataDictionary,
                  bool validate )
throw( InvalidMessage )
: FieldMap( FieldMap::create_allocator() ),
  m_header( get_allocator(), message_order( message_order::header ) ),
  m_trailer( get_allocator(), message_order( message_order::trailer ) ),
  m_status( 0 )
{
  setString( string, validate, &dataDictionary, &dataDictionary );
}

Message::Message( const std::string& string,
                  const DataDictionary& dataDictionary,
                  FieldMap::allocator_type& a, 
                  bool validate )
throw( InvalidMessage )
: FieldMap(a),
  m_header( a, message_order( message_order::header ) ),
  m_trailer( a, message_order( message_order::trailer ) ),
  m_status( 0 )
{

  setString( string, validate, &dataDictionary, &dataDictionary );
}

HEAVYUSE Message::Message( const std::string& string,
                  const DataDictionary& sessionDataDictionary,
                  const DataDictionary& applicationDataDictionary,
                  bool validate )
throw( InvalidMessage )
: FieldMap( FieldMap::create_allocator() ),
  m_header( get_allocator(), message_order( message_order::header ) ),
  m_trailer( get_allocator(), message_order( message_order::trailer ) ),
  m_status( 0 )
{
  if( isAdminMsg( string ) )
    setString( string, validate, &sessionDataDictionary, &sessionDataDictionary );
  else
    setString( string, validate, &sessionDataDictionary, &applicationDataDictionary );
}

Message::Message( const std::string& string,
                  const DataDictionary& sessionDataDictionary,
                  const DataDictionary& applicationDataDictionary,
                  FieldMap::allocator_type& a, 
                  bool validate )
throw( InvalidMessage )
: FieldMap(a),
  m_header( a, message_order( message_order::header ) ),
  m_trailer( a, message_order( message_order::trailer ) ),
  m_status( 0 )
{
  if( isAdminMsg( string ) )
    setString( string, validate, &sessionDataDictionary, &sessionDataDictionary );
  else
    setString( string, validate, &sessionDataDictionary, &applicationDataDictionary );
}

bool Message::InitializeXML( const std::string& url )
{
  try
  {
    std::auto_ptr<DataDictionary> p =
      std::auto_ptr<DataDictionary>(new DataDictionary(url));
    s_dataDictionary = p;
    return true;
  }
  catch( ConfigError& )
  { return false; }
}

void Message::reverseRoute( const Header& header )
{
  // required routing tags
  BeginString beginString;
  SenderCompID senderCompID;
  TargetCompID targetCompID;

  m_header.removeField( beginString.getField() );
  m_header.removeField( senderCompID.getField() );
  m_header.removeField( targetCompID.getField() );

  if( header.isSetField( beginString ) )
  {
    header.getField( beginString );
    if( beginString.hasValue() )
      m_header.setField( beginString );

    OnBehalfOfLocationID onBehalfOfLocationID;
    DeliverToLocationID deliverToLocationID;

    m_header.removeField( onBehalfOfLocationID.getField() );
    m_header.removeField( deliverToLocationID.getField() );

    if( beginString >= BeginString_FIX41 )
    {
      if( header.isSetField( onBehalfOfLocationID ) )
      {
        header.getField( onBehalfOfLocationID );
        if( onBehalfOfLocationID.hasValue() )
          m_header.setField( DeliverToLocationID( onBehalfOfLocationID ) );
      }

      if( header.isSetField( deliverToLocationID ) )
      {
        header.getField( deliverToLocationID );
        if( deliverToLocationID.hasValue() )
          m_header.setField( OnBehalfOfLocationID( deliverToLocationID ) );
      }
    }
  }

  if( header.isSetField( senderCompID ) )
  {
    header.getField( senderCompID );
    if( senderCompID.hasValue() )
      m_header.setField( TargetCompID( senderCompID ) );
  }

  if( header.isSetField( targetCompID ) )
  {
    header.getField( targetCompID );
    if( targetCompID.hasValue() )
      m_header.setField( SenderCompID( targetCompID ) );
  }

  // optional routing tags
  OnBehalfOfCompID onBehalfOfCompID;
  OnBehalfOfSubID onBehalfOfSubID;
  DeliverToCompID deliverToCompID;
  DeliverToSubID deliverToSubID;

  m_header.removeField( onBehalfOfCompID.getField() );
  m_header.removeField( onBehalfOfSubID.getField() );
  m_header.removeField( deliverToCompID.getField() );
  m_header.removeField( deliverToSubID.getField() );

  if( header.isSetField( onBehalfOfCompID ) )
  {
    header.getField( onBehalfOfCompID );
    if( onBehalfOfCompID.hasValue() )
      m_header.setField( DeliverToCompID( onBehalfOfCompID ) );
  }

  if( header.isSetField( onBehalfOfSubID ) )
  {
    header.getField( onBehalfOfSubID );
    if( onBehalfOfSubID.hasValue() )
      m_header.setField( DeliverToSubID( onBehalfOfSubID ) );
  }

  if( header.isSetField( deliverToCompID ) )
  {
    header.getField( deliverToCompID );
    if( deliverToCompID.hasValue() )
      m_header.setField( OnBehalfOfCompID( deliverToCompID ) );
  }

  if( header.isSetField( deliverToSubID ) )
  {
    header.getField( deliverToSubID );
    if( deliverToSubID.hasValue() )
      m_header.setField( OnBehalfOfSubID( deliverToSubID ) );
  }
}

std::string& HEAVYUSE
Message::toString( const FieldCounter& c, std::string& str ) const
{
  const int checkSumField = c.getCheckSumTag();
  const int bodyLengthField = c.getBodyLengthTag();
  const int csumPayloadLength = CheckSumConvertor::MaxValueSize + 1;
  const int csumTagLength = Util::PositiveInt::numDigits(checkSumField) + 1;

  int l = c.getBodyLength();
  l += m_header.setField(IntField::Pack(bodyLengthField, l)).getLength()
       + csumTagLength + csumPayloadLength + c.getBeginStringLength();

  str.clear();
  str.reserve(l);

  if( getStatusBit( hint_no_reuse ) && checkSumField == FIELD::CheckSum )
  {
    FieldBase& f = m_trailer.setField(CheckSumField::Pack(checkSumField, 0));

    m_trailer.serializeTo( 
      FieldMap::serializeTo(
        m_header.serializeTo( str ) ) );

    l -= csumPayloadLength;

    char* p = const_cast<char*>( String::c_str(str) );
    int32_t csum = Util::CharBuffer::checkSum( p, l - csumTagLength ) % 256;
    p += l;
    // NOTE: We can modify this data in place even for Copy-on-Write strings
    //       as the string is constructed locally
    CheckSumConvertor::generate(p, (unsigned char) csum );
    f.setPacked( StringField::Pack( checkSumField, p, 3 ) );
  }
  else
  {
    m_trailer.setField(CheckSumField::Pack(checkSumField,
                                           checkSum(checkSumField)));
    m_trailer.serializeTo( 
      FieldMap::serializeTo(
        m_header.serializeTo( str ) ) );
  }
  return str;
}

std::string Message::toXML() const
{
  std::string str;
  return toXML( str );
}

std::string& Message::toXML( std::string& str ) const
{
  std::stringstream stream;
  stream << "<message>"                         << std::endl
         << std::setw(2) << " " << "<header>"   << std::endl
         << toXMLFields(getHeader(), 4)
         << std::setw(2) << " " << "</header>"  << std::endl
         << std::setw(2) << " " << "<body>"     << std::endl
         << toXMLFields(*this, 4)
         << std::setw(2) << " " << "</body>"    << std::endl
         << std::setw(2) << " " << "<trailer>"  << std::endl
         << toXMLFields(getTrailer(), 4)
         << std::setw(2) << " " << "</trailer>" << std::endl
         << "</message>";

  return str = stream.str();
}

std::string Message::toXMLFields(const FieldMap& fields, int space) const
{
  std::stringstream stream;
  FieldMap::iterator i;
  std::string name;
  for(i = fields.begin(); i != fields.end(); ++i)
  {
    int field = i->first;
    std::string value = i->second.getString();

    stream << std::setw(space) << " " << "<field ";
    if(s_dataDictionary.get() && s_dataDictionary->getFieldName(field, name))
    {
      stream << "name=\"" << name << "\" ";
    }
    stream << "number=\"" << field << "\"";
    if(s_dataDictionary.get()
       && s_dataDictionary->getValueName(field, value, name))
    {
      stream << " enum=\"" << name << "\"";
    }
    stream << ">";
    stream << "<![CDATA[" << value << "]]>";
    stream << "</field>" << std::endl;
  }

  FieldMap::g_iterator j;
  for(j = fields.g_begin(); j != fields.g_end(); ++j)
  {
    FieldMap::g_item_const_iterator k, kend = j->second.end();
    for(k = j->second.begin(); k != kend; ++k)
    {
      stream << std::setw(space) << " " << "<group>" << std::endl
             << toXMLFields(*(*k), space+2)
             << std::setw(space) << " " << "</group>" << std::endl;
    }
  }

  return stream.str();
}

inline void HEAVYUSE Message::extractField ( Message::FieldReader& reader,
    const DataDictionary* pSessionDD, const DataDictionary* pAppDD,
    const Group* pGroup)
{
  int field = reader.scan();

  if ( LIKELY(pSessionDD && pSessionDD->isDataField(field)) ||
             (pAppDD && pAppDD != pSessionDD && pAppDD->isDataField(field)) )
  {
    // length field is 1 less except for Signature
    long lenField = (field != FIELD::Signature) ? (field - 1) : FIELD::SignatureLength;
    
    if ( pGroup && pGroup->isSetField( lenField ) )
    {
      const std::string& fieldLength = pGroup->getField( lenField );
      if ( !IntConvertor::parse( fieldLength, lenField ) )
        throw InvalidMessage("Malformed field length for field " +
                                   IntConvertor::convert(field) );
      reader.pos( lenField );
    }
    else if ( isSetField( lenField ) )
    {
      const std::string& fieldLength = getField( lenField );
      if ( !IntConvertor::parse( fieldLength, lenField ) )
        throw InvalidMessage("Malformed field length for field " +
                                   IntConvertor::convert(field) );
      reader.pos( lenField );
    }
  }
}

static const DataDictionary::FieldToGroup::key_type group_key_header_ =
       std::make_pair(0, String::value_type("_header_") );
static const DataDictionary::FieldToGroup::key_type group_key_null_header_ =
       std::make_pair(0, String::value_type() );
static const DataDictionary::FieldToGroup::key_type group_key_trailer_ =
       std::make_pair(0, String::value_type("_trailer_") );
static const DataDictionary::FieldToGroup::key_type group_key_null_trailer_ =
       std::make_pair(0, String::value_type() );

void HEAVYUSE Message::setString( const std::string& str,
                         bool doValidation,
                         const DataDictionary* pSessionDataDictionary,
                         const DataDictionary* pApplicationDataDictionary )
throw( InvalidMessage )
{
  DataDictionary::FieldToGroup::key_type header_key ( pSessionDataDictionary
                                         ? group_key_header_ : group_key_null_header_ );
  DataDictionary::FieldToGroup::key_type trailer_key ( pSessionDataDictionary
                                         ? group_key_trailer_ : group_key_null_trailer_ );
  DataDictionary::FieldToGroup::key_type msg_key;
  const BodyLength* pBodyLength = NULL;
  const FieldBase* p;
  FieldReader reader(str);

  clear();

  field_type type = header;

  if( doValidation )
  {
    extractField( reader, pSessionDataDictionary, pApplicationDataDictionary );
    if ( reader.getField() != FIELD::BeginString )
      throw InvalidMessage("BeginString out of order");
  
    p = &( reader >> m_header );
    if ( pSessionDataDictionary )
    {
      header_key.first = p->getField();
      setGroup( reader, header_key, getHeader(), *pSessionDataDictionary );
    }
  
    if ( reader )
    {
      extractField( reader, pSessionDataDictionary, pApplicationDataDictionary );
      if ( reader.getField() != FIELD::BodyLength )
        throw InvalidMessage("BodyLength out of order");
  
      pBodyLength = (const BodyLength*)(p = &( reader >> m_header ));
      if ( pSessionDataDictionary ) 
      {
        header_key.first = p->getField();
        setGroup( reader, header_key, getHeader(), *pSessionDataDictionary );
      }
  
      if ( reader )
      {
        extractField( reader, pSessionDataDictionary, pApplicationDataDictionary );
        if ( reader.getField() != FIELD::MsgType )
          throw InvalidMessage("MsgType out of order");
  
        p = &( reader >> m_header );
  
        if ( pApplicationDataDictionary )
          msg_key.second = p->forString( String::RvalFunc() );
  
        if ( pSessionDataDictionary )
        {
          header_key.first = p->getField();
          setGroup( reader, header_key, getHeader(), *pSessionDataDictionary );
        }
      }
    }
  }

  while ( reader )
  {
    extractField( reader, pSessionDataDictionary, pApplicationDataDictionary );

    if ( isHeaderField( reader.getField(), pSessionDataDictionary ) )
    {
      if ( type != header )
      {
        if ( m_field == 0 ) m_field = reader.getField();
        setStatusBit( invalid_field );
      }

      p = &( reader >> m_header );
      if ( pSessionDataDictionary )
      {
        header_key.first = p->getField();
        setGroup( reader, header_key, getHeader(), *pSessionDataDictionary );
      }

      if ( pApplicationDataDictionary && p->getField() == FIELD::MsgType )
        msg_key.second = p->forString( String::RvalFunc() );
    }
    else if ( isTrailerField( reader.getField(), pSessionDataDictionary ) )
    {
      type = trailer;

      p = &( reader >> m_trailer );
      if ( pSessionDataDictionary )
      {
        trailer_key.first = p->getField();
        setGroup( reader, trailer_key, getTrailer(), *pSessionDataDictionary );
      }
    }
    else
    {
      if ( type == trailer )
      {
        if ( m_field == 0 ) m_field = reader.getField();
        setStatusBit( invalid_field );
      }

      type = body;

      p = &( reader >> *this );
      if ( pApplicationDataDictionary )
      {
        msg_key.first = p->getField();
        setGroup( reader, msg_key, *this, *pApplicationDataDictionary );
      }
    }
  }

  if ( doValidation )
    validate(pBodyLength);
}

void Message::setGroup( const std::string& msg,
                        const FieldBase& field, const std::string& str,
                        std::string::size_type& pos, FieldMap& map,
                        const DataDictionary& dataDictionary )
{
  FieldReader reader(str, pos);
  DataDictionary::FieldToGroup::key_type key( field.getField(), msg );
  setGroup( reader, key, map, dataDictionary );
  pos = reader.pos() - String::c_str(str);
}

void Message::setGroup( Message::FieldReader& reader,
                        DataDictionary::FieldToGroup::key_type& key,
                        FieldMap& map, const DataDictionary& dataDictionary )
{
  int delim, group = key.first;
  const DataDictionary* pDD = 0;
  if ( dataDictionary.getGroup( key, delim, pDD ) )
  {
    std::auto_ptr<Group> pGroup;
  
    while ( reader )
    {
      const char* pos = reader.pos();
      extractField( reader, &dataDictionary, &dataDictionary, pGroup.get() );
      if ( // found delimiter
           (reader.getField() == delim)
           // no delimiter, but field belongs to group or already processed
           || ((pGroup.get() == 0 || pGroup->isSetField(reader.getField())) &&
                pDD->isField(reader.getField())) )
      {
        if ( pGroup.get() )
        {
          map.addGroupPtr( group, pGroup.release(), false );
        }
        pGroup.reset( new Group( reader.getField(), delim, pDD->getOrderedFields() ) );
      }
      else if ( !pDD->isField( reader.getField() ) )
      {
        if ( pGroup.get() )
        {
          map.addGroupPtr( group, pGroup.release(), false );
        }
        reader.rewind( pos );
        return ;
      }
  
      if ( !pGroup.get() ) return ;
      reader >> *pGroup;
      key.first = reader.getField();
      setGroup( reader, key, *pGroup, *pDD );
    }
  }
}

bool Message::setStringHeader( const std::string& str )
{
  clear();

  FieldReader reader(str);
  int count = 0;
  while ( reader )
  {
    extractField( reader );
    if ( count < 3 && headerOrder[ count++ ] != reader.getField() )
      return false;

    if ( isHeaderField( reader.getField() ) )
       reader >> m_header;
    else break;
  }
  return true;
}

ALIGN_DECL_DEFAULT const int Message::HeaderFieldSet::m_fields[] =
{
FIELD::BeginString,
FIELD::BodyLength,
FIELD::MsgType,
FIELD::SenderCompID,
FIELD::TargetCompID,
FIELD::OnBehalfOfCompID,
FIELD::DeliverToCompID,
FIELD::SecureDataLen,
FIELD::MsgSeqNum,
FIELD::SenderSubID,
FIELD::SenderLocationID,
FIELD::TargetSubID,
FIELD::TargetLocationID,
FIELD::OnBehalfOfSubID,
FIELD::OnBehalfOfLocationID,
FIELD::DeliverToSubID,
FIELD::DeliverToLocationID,
FIELD::PossDupFlag,
FIELD::PossResend,
FIELD::SendingTime,
FIELD::OrigSendingTime,
FIELD::XmlDataLen,
FIELD::XmlData,
FIELD::MessageEncoding,
FIELD::LastMsgSeqNumProcessed,
FIELD::OnBehalfOfSendingTime,
FIELD::ApplVerID,
FIELD::CstmApplVerID,
FIELD::NoHops,
0
};

ALIGN_DECL_DEFAULT Message::HeaderFieldSet Message::headerFieldSet;

SessionID Message::getSessionID( const std::string& qualifier ) const
throw( FieldNotFound )
{
  BeginString beginString;
  SenderCompID senderCompID;
  TargetCompID targetCompID;

  getHeader().getField( beginString );
  getHeader().getField( senderCompID );
  getHeader().getField( targetCompID );

  return SessionID( beginString, senderCompID, targetCompID, qualifier );
}

void Message::setSessionID( const SessionID& sessionID )
{
  getHeader().setField( sessionID.getBeginString() );
  getHeader().setField( sessionID.getSenderCompID() );
  getHeader().setField( sessionID.getTargetCompID() );
}

void Message::validate(const BodyLength* pBodyLength)
{
  if ( pBodyLength )
  {
    try
    {
      if ( *pBodyLength != bodyLength() )
      {
        std::stringstream text;
        text << "Expected BodyLength=" << bodyLength()
             << ", Received BodyLength=" << (int)*pBodyLength;
        throw InvalidMessage(text.str());
      }
  
      const CheckSum& aCheckSum = FIELD_GET_REF( m_trailer, CheckSum );
  
      if ( aCheckSum != checkSum() )
      {
        std::stringstream text;
        text << "Expected CheckSum=" << checkSum()
             << ", Received CheckSum=" << (int)aCheckSum;
        throw InvalidMessage(text.str());
      }
    }
    catch ( FieldNotFound& )
    {
      throw InvalidMessage("CheckSum missing");
    }
    catch ( IncorrectDataFormat& )
    {
      throw InvalidMessage("BodyLength or Checksum has wrong format");
    }
  }
  else
    throw InvalidMessage("BodyLength missing");
}

}
