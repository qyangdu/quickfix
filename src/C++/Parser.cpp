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
static inline const char* memchr2(const char* p, char c1, char c2, int n) {
        union {
                char     c[2];
                uint16_t u;
        } u = { { c1, c2 } };
        for (int i = 0, l = n - 2; i <= l; i++, p++) {
                if (*(const uint16_t*)p == u.u )
                        return p;
        }
        return NULL;
}

std::size_t Parser::extractLength( const char* msg, std::size_t sz )
throw( MessageParseError )
{
  if( sz > 3 )
  {
    const char* p = Util::CharBuffer::memmem(msg, sz, "\0019=", 3);
    if( p )
    {
      std::size_t startPos = p - msg + 3;
      p = (const char*)::memchr( p + 3, '\001', sz - startPos);
      if( p )
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
  std::size_t bufSize = String::length(m_buffer);

  if( bufSize > 2 )
  {
    const char* buf = String::c_str(m_buffer);
    const char* p = memchr2(buf, '8', '=', bufSize);

    if( p )
    {
      std::size_t msgPos = p - buf;
      std::size_t length = msgPos + 2;

      try
      {
        if( (length = extractLength( (buf = p), bufSize - msgPos )) )
        {
          std::size_t checksumOffset = msgPos + length;
          if( bufSize >= (checksumOffset + 3) )
          {
            p = Util::CharBuffer::memmem( p + length - 1,
                            bufSize - checksumOffset + 1, "\00110=", 4 );
            if( p )
            {
              p += 4;
              p = (const char*)::memchr( p, '\001', bufSize - (p - buf));
              if( p )
              {
                length = (p - buf) + 1;
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
