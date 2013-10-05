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

#include <string.h>
#ifdef _MSC_VER
#include "stdafx.h"
#else
#include "config.h"
#endif

#include "Parser.h"
#include "Utility.h"
#include "FieldConvertors.h"
#include <algorithm>

#ifdef HAVE_BOOST
#include <boost/spirit/include/qi_numeric.hpp>
#endif

namespace FIX
{

std::size_t Parser::extractLength( const char* msg, std::size_t sz )
throw( MessageParseError )
{
  if( LIKELY(sz > 3) )
  {
    const char* p = Util::CharBuffer::memmem(msg, sz, "\0019=", 3);
    if( LIKELY(NULL != p) )
    {
      std::size_t startPos = p - msg + 3;
      p = (const char*)::memchr( p + 3, '\001', sz - startPos);
      if( LIKELY(NULL != p) )
      {
        int length = 0;
        std::size_t endPos = p - msg;
    
        if( UnsignedConvertor::parse(msg + startPos, p, length) )
          return endPos + 1 + length;

        throw MessageParseError();
      }
    }
  }
  return 0;
}

bool Parser::readFixMessage( std::string& str )
throw( MessageParseError )
{
  const std::size_t bufSize = String::length(m_buffer);

  if( LIKELY(bufSize > 2) )
  {
    Util::CharBuffer::Fixed<2> beginStringTag = { { '8', '=' } };

    const char* buf = String::c_str(m_buffer);
    const char* p = Util::CharBuffer::find( beginStringTag, buf, bufSize );

    if( LIKELY(NULL != p) )
    {
      std::size_t msgPos = p - buf;
      std::size_t length = msgPos + 2;

      try
      {
        if( LIKELY( 0 != (length = extractLength( p, bufSize - msgPos )) ) )
        {
          std::size_t checksumOffset = msgPos + length;
          if( LIKELY(bufSize >= (checksumOffset + 3)) )
          {
            Util::CharBuffer::Fixed<4> checkSumTag = { { '\001', '1', '0', '=' } };

            if( LIKELY(NULL !=
                      (p = Util::CharBuffer::find( checkSumTag, p + length - 1,
                                             bufSize - checksumOffset + 1 )) ) )
            {
              p += 4;
              p = (const char*)::memchr( p, '\001', bufSize - (p - buf));
              if( LIKELY(NULL != p) )
              {
                length = p - (buf + msgPos) + 1;
                if ( length == bufSize ) {
                  String::swap( str, m_buffer );
                  m_buffer.clear();
                } else {
                  m_buffer.substr( msgPos, length ).swap(str);
                  m_buffer.erase( 0, msgPos + length );
                }
                return true;
              }
            }
          }
        }
        if ( msgPos ) m_buffer.erase(0, msgPos);
      }
      catch( MessageParseError& e )
      {
        if( length > 0 )
          m_buffer.erase( 0, msgPos + length );
        else
          m_buffer.erase();
    
        throw e;
      }
    } 
  }
  return false;
}

}
