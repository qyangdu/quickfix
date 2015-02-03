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

Message::Message( SerializationHint hint, int fieldCountEstimate )
: FieldMap( FieldMap::create_allocator( fieldCountEstimate ) ),
  m_header( get_allocator(), message_order( message_order::header ) ),
  m_trailer( get_allocator(), message_order( message_order::trailer ) ),
  m_status( createStatus(serialized_once, hint == SerializedOnce) ) {}

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

HEAVYUSE Message::Message( const std::string& string,
                  const FIX::DataDictionary& sessionDataDictionary,
                  const FIX::DataDictionary& applicationDataDictionary,
                  FieldMap::allocator_type& allocator, bool validate )
  throw( InvalidMessage )
: FieldMap( allocator ),
  m_header( allocator, message_order( message_order::header ) ),
  m_trailer( allocator, message_order( message_order::trailer ) ),
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

  if( header.getFieldIfSet( beginString ) )
  {
    if( beginString.hasValue() )
      m_header.setField( beginString );

    OnBehalfOfLocationID onBehalfOfLocationID;
    DeliverToLocationID deliverToLocationID;

    m_header.removeField( onBehalfOfLocationID.getField() );
    m_header.removeField( deliverToLocationID.getField() );

    if( beginString >= BeginString_FIX41 )
    {
      if( header.getFieldIfSet( onBehalfOfLocationID ) )
      {
        if( onBehalfOfLocationID.hasValue() )
          m_header.setField( DeliverToLocationID( onBehalfOfLocationID ) );
      }

      if( header.getFieldIfSet( deliverToLocationID ) )
      {
        if( deliverToLocationID.hasValue() )
          m_header.setField( OnBehalfOfLocationID( deliverToLocationID ) );
      }
    }
  }

  if( header.getFieldIfSet( senderCompID ) )
  {
    if( senderCompID.hasValue() )
      m_header.setField( TargetCompID( senderCompID ) );
  }

  if( header.getFieldIfSet( targetCompID ) )
  {
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

  if( header.getFieldIfSet( onBehalfOfCompID ) )
  {
    if( onBehalfOfCompID.hasValue() )
      m_header.setField( DeliverToCompID( onBehalfOfCompID ) );
  }

  if( header.getFieldIfSet( onBehalfOfSubID ) )
  {
    if( onBehalfOfSubID.hasValue() )
      m_header.setField( DeliverToSubID( onBehalfOfSubID ) );
  }

  if( header.getFieldIfSet( deliverToCompID ) )
  {
    if( deliverToCompID.hasValue() )
      m_header.setField( OnBehalfOfCompID( deliverToCompID ) );
  }

  if( header.getFieldIfSet( deliverToSubID ) )
  {
    if( deliverToSubID.hasValue() )
      m_header.setField( OnBehalfOfSubID( deliverToSubID ) );
  }
}

namespace detail {
	class ProxyBuffer {

		typedef uint32_t word_type;
		Sg::sg_buf_t buf_;

		public:
			ProxyBuffer(std::string& s) {
				buf_ = IOV_BUF_INITIALIZER(const_cast<char*>( String::c_str(s) ), 0);
			}
			ProxyBuffer& append(const char* s, std::size_t len) {
				union { char* pc; word_type* pw; } dst = { (char*)IOV_BUF(buf_) + IOV_LEN(buf_) };
				union { const char* pc; const word_type* pw; } src = { s };
				IOV_LEN(buf_) += len;
                                for (;len >= sizeof(word_type); len -= sizeof(word_type)) *dst.pw++ = *src.pw++;
				while (len--) *dst.pc++ = *src.pc++;
				return *this;
			}
			ProxyBuffer& append(int field, int sz, const char* src, std::size_t len) {
                                char* dst = (char*)IOV_BUF(buf_) + IOV_LEN(buf_);
                                Util::Tag::generate(dst, field, sz);
                                dst[sz++] = '=';
                                IOV_LEN(buf_) += sz;
                                append(src, len);
                                IOV_LEN(buf_)++;
                                dst[sz + len] = '\001';
                                return *this;

			}
			char* data() const {
				return Sg::data<char*>(buf_);
			}
			int size() const {
				return Sg::size(buf_);
			}
	};
}

std::string& HEAVYUSE
Message::toString( const FieldCounter& c, std::string& str ) const
{
  char* p;
  const int checkSumField = c.getCheckSumTag();
  const int bodyLengthField = c.getBodyLengthTag();
  const int csumPayloadLength = CheckSumConvertor::MaxValueSize + 1;
  const int csumTagLength = Util::PositiveInt::numDigits(checkSumField) + 1;

  int l = c.getBodyLength();
  l += m_header.setField(IntField::Pack(bodyLengthField, l)).getLength()
       + csumTagLength + csumPayloadLength + c.getBeginStringLength();

  str.clear();
#ifndef NO_PROXY_BUFFER
  str.resize(l);
  detail::ProxyBuffer sbuf(str);
#else
  str.reserve(l);
  std::string& sbuf = str;
#endif

  // NOTE: We can modify data in place even for Copy-on-Write strings
  //       as the string is constructed locally
  if( getStatusBit( serialized_once ) && checkSumField == FIELD::CheckSum )
  {
    FieldBase& f = m_trailer.setField(CheckSumField::Pack(checkSumField, 0));

    m_trailer.serializeTo( 
      FieldMap::serializeTo(
        m_header.serializeTo( sbuf ) ) );

    l -= csumPayloadLength;

    p = (char*)sbuf.data();
    int32_t csum = Util::CharBuffer::checkSum( p, l - csumTagLength ) & 255;
    p += l;
    CheckSumConvertor::generate(p, (unsigned char) csum );
    f.setPacked( StringField::Pack( checkSumField, p, 3 ) );
  }
  else
  {
    m_trailer.setField(CheckSumField::Pack(checkSumField,
                                           checkSum(checkSumField)));
    m_trailer.serializeTo( 
      FieldMap::serializeTo(
        m_header.serializeTo( sbuf ) ) );
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

inline bool Message::extractFieldDataLength( Message::FieldReader& reader, const Group* pGroup, int field )
{
  // length field is 1 less except for Signature
  int lenField = (field != FIELD::Signature) ? (field - 1) : FIELD::SignatureLength;
  const FieldBase* fieldPtr = (pGroup) ? pGroup->getFieldPtrIfSet( lenField ) : NULL;
  if ( fieldPtr || NULL != (fieldPtr = getFieldPtrIfSet( lenField ) ) )
  {
    const FieldBase::string_type& fieldLength = fieldPtr->getRawString();
    if ( IntConvertor::parse( fieldLength, lenField ) )
      reader.pos( lenField );
    else
    {
      setErrorStatusBit( incorrect_data_format, lenField );
      return false;
    }
  }
  return true;
}

inline bool HEAVYUSE Message::extractField ( Message::FieldReader& reader,
    const DataDictionary* pSessionDD, const DataDictionary* pAppDD,
    const Group* pGroup)
{
  const char* errpos = reader.scan();
  if( LIKELY(!errpos) )
  {
    int field = reader.getField();
    if ((LIKELY(!pSessionDD || !pSessionDD->isDataField(field)) &&
         LIKELY(!pAppDD || pAppDD == pSessionDD || !pAppDD->isDataField(field))) ||
         extractFieldDataLength( reader, pGroup, field) )
      return true;
  }
  else
    setErrorStatusBit( invalid_tag_format, (intptr_t)errpos );

  reader.skip();
  return false;
}

ALIGN_DECL_DEFAULT static const DataDictionary::FieldPresenceMap::key_type group_key_header_ =
       DataDictionary::FieldPresenceMap::key_type("_header_");
ALIGN_DECL_DEFAULT static const DataDictionary::FieldPresenceMap::key_type group_key_trailer_ =
       DataDictionary::FieldPresenceMap::key_type("_trailer_");

void HEAVYUSE
Message::setString( const char* data, std::size_t length,
                    bool doValidation,
                    const DataDictionary* pSessionDataDictionary,
                    const DataDictionary* pApplicationDataDictionary )
throw( InvalidMessage )
{
  DataDictionary::FieldPresenceMap::key_type msg_key;
  const BodyLength* pBodyLength = NULL;
  const FieldBase* p;
  FieldReader reader( data, length );

  clear();

  field_type type = header;

  if( reader && 
      extractField( reader, pSessionDataDictionary, pApplicationDataDictionary ) )
  {
    if( LIKELY(reader.getField() == FIELD::BeginString) )
    {
      p = &( reader >> m_header );
      if ( pSessionDataDictionary )
        setGroup( reader, group_key_header_, p->getField(), m_header, *pSessionDataDictionary );
    }
    else
    {
      if( doValidation )
        throw InvalidMessage("BeginString field out of order");
      goto loop;
    }
  }
  
  if ( reader &&
       extractField( reader, pSessionDataDictionary, pApplicationDataDictionary ) )
  {
    if( LIKELY(reader.getField() == FIELD::BodyLength) )
    {
      pBodyLength = (const BodyLength*)(p = &( reader >> m_header ));
      if ( pSessionDataDictionary ) 
        setGroup( reader, group_key_header_, p->getField(), m_header, *pSessionDataDictionary );
    }
    else
    {
      if( doValidation )
        throw InvalidMessage("BodyLength field out of order");
      goto loop;
    }
  }
  
  if ( reader &&
       extractField( reader, pSessionDataDictionary, pApplicationDataDictionary ) )
  {
    if ( LIKELY(reader.getField() == FIELD::MsgType) )
    {
      p = &( reader >> m_header );

      if ( pApplicationDataDictionary )
        msg_key = p->forString( String::RvalFunc() );

      if ( pSessionDataDictionary )
        setGroup( reader, group_key_header_, p->getField(), m_header, *pSessionDataDictionary );
    }
    else 
    {
      if( doValidation )
        throw InvalidMessage("MsgType field out of order");
      goto loop;
    }
  }

  while ( reader )
  {
    if( extractField( reader, pSessionDataDictionary, pApplicationDataDictionary ) )
    {
loop:
      int field = reader.getField();
      if ( isHeaderField( field, pSessionDataDictionary ) )
      {
        if ( LIKELY(type == header) )
        {
          if ( field == FIELD::SenderCompID || field == FIELD::TargetCompID )
            setStatusBit( ( field == FIELD::SenderCompID )
                          ? has_sender_comp_id : has_target_comp_id );
        }
        else
          setErrorStatusBit( tag_out_of_order, field );
  
        p = &( reader >> m_header );
        if ( pSessionDataDictionary )
          setGroup( reader, group_key_header_, p->getField(), m_header, *pSessionDataDictionary );
  
        if ( pApplicationDataDictionary && p->getField() == FIELD::MsgType )
          msg_key = p->forString( String::RvalFunc() );
      }
      else if ( LIKELY(!isTrailerField( field, pSessionDataDictionary )) )
      {
        if ( type == trailer )
          setErrorStatusBit( tag_out_of_order, field );
  
        type = body;
  
        p = &( reader >> *this );
        if ( pApplicationDataDictionary )
          setGroup( reader, msg_key, p->getField(), *this, *pApplicationDataDictionary );
      }
      else
      {
        type = trailer;
  
        p = &( reader >> m_trailer );
        if ( pSessionDataDictionary )
          setGroup( reader, group_key_trailer_, p->getField(), m_trailer, *pSessionDataDictionary );
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
  setGroup( reader, String::CopyFunc()(msg), field.getField(), map, dataDictionary );
  pos = reader.pos() - String::c_str(str);
}

void Message::setGroup( Message::FieldReader& reader,
			const DataDictionary::FieldPresenceMap::key_type& msg, const int group,
                        FieldMap& map, const DataDictionary& dataDictionary )
{
  int delim;
  const DataDictionary* pDD = 0;
  if ( LIKELY(!dataDictionary.getGroup( msg, group, delim, pDD )) ) return;

  std::auto_ptr<Group> pGroup;
  while ( reader )
  {
    const char* pos = reader.pos();
    if( extractField( reader, &dataDictionary, &dataDictionary, pGroup.get() ) )
    {
      if( // found delimiter
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
      setGroup( reader, msg, reader.getField(), *pGroup, *pDD );
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
    if( extractField( reader ) )
    {
      if( count < 3 && headerOrder[ count++ ] != reader.getField() )
        return false;

      if( isHeaderField( reader.getField() ) )
        reader >> m_header;
      else break;
    }
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
