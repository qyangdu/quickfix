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

#ifndef FIX_MESSAGESORTERS_H
#define FIX_MESSAGESORTERS_H

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#else
#include "Config.h"
#endif

#include "AtomicCount.h"
#include "FieldNumbers.h"
#include <stdarg.h>
#include <functional>
#include <map>

namespace FIX
{
/// Sorts fields in correct header order.
struct header_order
{
  static inline bool NOTHROW compare( const int x, const int y )
  {
    int orderedX = getOrderedPosition( x );
    int orderedY = getOrderedPosition( y );

    return orderedX < orderedY;
  }

  static inline int NOTHROW getOrderedPosition( const int field )
  {
    int s = -3 * (field == FIX::FIELD::BeginString) |
            -2 * (field == FIX::FIELD::BodyLength) |
            -1 * (field == FIX::FIELD::MsgType);
    return (s == 0) ? field : s;
  }
};

/// Sorts fields in correct trailer order.
struct trailer_order
{
  static inline bool NOTHROW compare( const int x, const int y )
  {
    return ((x < y) || (y == FIELD::CheckSum)) && (x != FIELD::CheckSum);
  }
};

/// Sorts fields in correct group order
struct group_order
{
  static inline bool NOTHROW compare( const int x, const int y, int* order, int largest )
  {
    int iX = (x <= largest) ? order[x] : x;
    int iY = (y <= largest) ? order[y] : y;

    return iX < iY;
  }
};

typedef std::less < int > normal_order;

/**
 * Sorts fields in header, normal, or trailer order.
 *
 * Used as a dynamic sorter to create Header, Trailer, and Message
 * FieldMaps while maintaining the same base type.
 */
struct message_order
{
  class group_order_array
  {
    class storage
    {
      public:

        static inline
        int* allocate( std::size_t largest, AtomicCount::value_type v )
        {
          std::size_t bufsz = sizeof(storage) + sizeof(int) * (largest + 1);
          storage* p = new ((storage*) new char[bufsz]) storage( v );
          return reinterpret_cast<int*>(p + 1);
        }
        static inline void deallocate( int* buffer )
        {
          storage* p = get(buffer);
          p->~storage();
          delete [] reinterpret_cast<char*>(p);
        }
  
        static inline AtomicCount& counter( int* buffer )
        { return get(buffer)->m_count; }

      private:

        storage( AtomicCount::value_type v = 0 )
        : m_count(v) {}

        storage( const storage& );
        storage& operator = ( const storage& );

        static inline storage* get( int* buffer )
        { return reinterpret_cast<storage*>(buffer) - 1; }

        AtomicCount m_count;
    };

    public:

      group_order_array() : m_buffer(NULL) {}

      group_order_array(const group_order_array& rhs)
      : m_buffer( rhs.acquire() )
      {}
 
      ~group_order_array()
      { release(); }
 
      group_order_array& operator = (const group_order_array& rhs)
      {
        if( &rhs != this )
        {
          release();
          m_buffer = rhs.acquire();
        }
        return *this;
      }
 
      bool empty() const
      { return m_buffer == NULL; }
 
      operator int * () const
      { return m_buffer; }
 
      static inline group_order_array create( std::size_t largest )
      {
        if( largest )
        {
           int* p = storage::allocate( largest, 1 );
           for( std::size_t i = 0; i < largest; i++ )
             p[ i ] = i; // >= 0 by default
           return p;
        }
        return NULL;
      }

    private:

      group_order_array(int* buffer) : m_buffer(buffer)
      {}

      int* acquire() const
      {
        if( !empty() )
          ++storage::counter(m_buffer);
        return m_buffer;
      }

      void release()
      {
        if( !empty() && 0 == --storage::counter(m_buffer) )
        {
          storage::deallocate(m_buffer);
          m_buffer = NULL;
        }
      }

      int* m_buffer;
  };

public:
  enum cmp_mode { header, trailer, normal, group };

  message_order( cmp_mode mode = normal )
  : m_mode( mode ), m_delim ( 0 ), m_largest( 0 ) {}
  message_order( int first, ... );
  message_order( const int order[] );
  message_order( const message_order& copy )
  : m_mode( copy.m_mode )
  , m_delim( copy.m_delim )
  , m_largest( copy.m_largest )
  , m_groupOrder( copy.m_groupOrder )
  {}

  inline bool NOTHROW operator() ( const int x, const int y ) const
  {
    switch ( m_mode )
    {
      case header:
      return header_order::compare( x, y );
      case trailer:
      return trailer_order::compare( x, y );
      case group:
      return group_order::compare( x, y, m_groupOrder, m_largest );
      case normal: default:
      return x < y;
    }
  }

  inline message_order& NOTHROW operator=( const message_order& rhs )
  {
    m_mode = rhs.m_mode;
    m_delim = rhs.m_delim;
    m_largest = rhs.m_largest;
    m_groupOrder = rhs.m_groupOrder;
    return *this;
  }

  operator bool() const { return !m_groupOrder.empty(); }

private:
  cmp_mode m_mode;
  int m_delim;
  int m_largest;
  group_order_array m_groupOrder;
};
}

#endif //FIX_MESSAGESORTERS_H

