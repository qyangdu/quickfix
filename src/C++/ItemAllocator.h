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

#ifndef ITEM_ALLOCATOR_H
#define ITEM_ALLOCATOR_H

#include "Utility.h"

namespace FIX
{
  // Caching allocator for node-based containers
  class ItemStore
  {
    public:
      static const int MaxCapacity = 64;

      static const int SharedCapacity = MaxCapacity;
      static const int DefaultCapacity = MaxCapacity / 2;

      struct Buffer {
		typedef Util::BitSet<MaxCapacity> Bitmap;

		unsigned        m_shared;
		unsigned 	m_item_size;
		std::size_t     m_size;
		Bitmap          m_bitmap;
      };

      ItemStore(Buffer* b = NULL) : m_buffer(b) {}
      ItemStore(ItemStore const& a)
        : m_buffer( a.m_buffer )
      {
        if ( m_buffer ) m_buffer->m_shared++;
      }
      void clear()
      {
        if ( m_buffer )
          m_buffer->m_item_size = 0;
      }

      std::size_t item_size() const
      {
        return (m_buffer) ? m_buffer->m_item_size : 0;
      }

      static Buffer* buffer(std::size_t size, unsigned short referenced = 0)
      {
	Buffer* h = (Buffer*)
		(ALLOCATOR<unsigned char>().allocate(size + sizeof(Buffer)));
        h->m_shared = referenced + 1;
	h->m_item_size = 0;
	h->m_size = size;
	return h;
      }

   protected:
     Buffer* m_buffer;
  };

  template<typename T>
  class ItemAllocator : public ItemStore
  {
    public : 

      //    typedefs
      typedef T value_type;
      typedef value_type* pointer;
      typedef const value_type* const_pointer;
      typedef value_type& reference;
      typedef const value_type& const_reference;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

    public : 
      //    convert an allocator<T> to allocator<U>
      template<typename U>
      struct rebind {
          typedef ItemAllocator<U> other;
      };
  
    public : 
      inline ItemAllocator() {}
      inline ItemAllocator(Buffer* b) : ItemStore(b) {}

      inline explicit ItemAllocator(ItemAllocator const& a)
       : ItemStore(a) {}

      template<typename U>
      inline ItemAllocator(ItemAllocator<U> const& u)
       : ItemStore(u) {}

      inline ~ItemAllocator() // clean up here, not a virtual dtor
      {
        if ( m_buffer && --m_buffer->m_shared == 0 )
          ALLOCATOR<unsigned char>().deallocate((unsigned char*)m_buffer, 0);
      }
  
      //    address
      inline pointer address(reference r) { return &r; }
      inline const_pointer address(const_reference r) { return &r; }
  
      //    memory allocation
      inline pointer HEAVYUSE allocate(size_type cnt, 
                     typename std::allocator<void>::const_pointer = 0)
      { 
        if ( cnt == 1 )
        {
restart:
          if ( m_buffer )
          {
            unsigned char (*arena)[sizeof(value_type)] =
                          (unsigned char(*)[sizeof(value_type)])(m_buffer + 1);
            if ( m_buffer->m_item_size )
            {
mapped:
              if ( m_buffer->m_item_size >= sizeof(value_type) )
              {
                Buffer::Bitmap& bitmap = m_buffer->m_bitmap;
                unsigned char slot = bitmap._Find_first();
                if ( slot != bitmap.size() )
                {
                  bitmap.reset(slot);
	          return (value_type*)(arena[slot]);
                }
              }
            }
            else
            {
              unsigned elements = m_buffer->m_size / sizeof(value_type);
              if ( elements )
              {
                Buffer::Bitmap& bitmap = m_buffer->m_bitmap;
		elements = (std::min)(elements, (unsigned)bitmap.size());
                bitmap.set();
                bitmap >>= (bitmap.size() - elements);
                m_buffer->m_item_size = sizeof(value_type);
                goto mapped;
              }
              else
              {
                m_buffer->m_bitmap.reset();
              }
            }
          }
          else
          {
            m_buffer = buffer(MaxCapacity * sizeof(value_type));
            goto restart;
          }
        }
        return m_allocator.allocate(cnt); 
      }

      inline void deallocate(pointer p, size_type t)
      { 
        if (m_buffer && (unsigned char*)p > ((unsigned char*)m_buffer) &&
                        (unsigned char*)p < ((unsigned char*)m_buffer +
                                        sizeof(Buffer) + m_buffer->m_size))
        {
          unsigned char slot = p - (value_type*)(m_buffer + 1);
          m_buffer->m_bitmap.set(slot);
          return;
        }
        m_allocator.deallocate(p, t); 
      }
  
      //    size
      inline size_type max_size() const
      { return (std::numeric_limits<size_type>::max)() / sizeof(value_type); }
  
      //    construction/destruction
      inline void construct(pointer p, const value_type& t)
      { new(p) value_type(t); }
      inline void destroy(pointer p)
      { p->~value_type(); }
  
      inline bool operator==(ItemAllocator const&)
      { return true; }
      inline bool operator!=(ItemAllocator const& a)
      { return !operator==(a); }

    private:
      typedef ALLOCATOR<value_type> Allocator;
      Allocator     m_allocator;
  };    //    end of class ItemAllocator 

} // namespace FIX

#endif
