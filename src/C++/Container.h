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

#ifndef FIX_CONTAINER_H
#define FIX_CONTAINER_H

#include <functional>
#include <iterator>
#include <utility>
#include <memory>

#include "Utility.h"

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

#ifdef HAVE_RDESTL
#include <rdestl/hash_map.h>
#include <rdestl/vector.h>
#endif

#ifdef HAVE_ULIB
#include <ulib/hash_open.h>
#endif

namespace FIX 
{
namespace Container {

// Minimal dictionary without erase capability
// based on open addressing hash table with power of 2 size,
// triangular sequence quadratic probing and cached key hashes.
// Optimized specializations for tag indices assume tags to be < 99999999.
template <typename Dictionary, typename Traits> class DictionaryBase {
public:

  typedef typename Traits::value_type value_type;
  typedef typename Traits::key_type key_type;
  typedef typename Traits::mapped_type mapped_type;
  typedef typename Traits::hasher hasher;
  typedef typename Traits::key_equal key_equal;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef value_type& reference;
  typedef value_type const& const_reference;

protected:
  typedef typename Traits::hash_type hash_type;
  typedef typename Traits::stored_type stored_type;

  static const int MaxLoadPercentage = 70; 

  static inline stored_type* first(stored_type* p) {
    do { if (!Traits::empty(p)) return p; } while(!Traits::last(p++));
    return NULL;
  }

  template <typename P> static inline P* next(P* p) {
    while (!Traits::last(p)) { if (!Traits::empty(++p)) return p; }
    return NULL;
  }

  inline Dictionary* dictionary() { return static_cast<Dictionary*>(this); }
  inline const Dictionary* dictionary() const { return static_cast<const Dictionary*>(this); }

public:
  class iterator {

    stored_type* m_p;
    public:
      typedef std::forward_iterator_tag iterator_category;

      typedef typename Traits::value_type value_type;
      typedef value_type* pointer;
      typedef value_type& reference;
      typedef std::ptrdiff_t difference_type;

      iterator(stored_type* p = NULL) : m_p(p) {}
      reference operator*() { return Traits::value(m_p); }
      pointer operator->() { return &Traits::value(m_p); }

      iterator& operator++() { m_p = next(m_p); return *this; }
      iterator operator++(int) { iterator c(m_p); ++(*this); return c; }
      bool operator==( const iterator& rhs ) const { return m_p == rhs.m_p; }
      bool operator!=( const iterator& rhs ) const { return m_p != rhs.m_p; }
  };

  class const_iterator {

    const stored_type* m_p;
    public:
      typedef std::forward_iterator_tag iterator_category;

      typedef const typename Traits::value_type value_type;
      typedef value_type* pointer;
      typedef value_type& reference;
      typedef std::ptrdiff_t difference_type;

      const_iterator(stored_type* p = NULL) : m_p(p) {}
      reference operator*() const { return Traits::value(m_p); }
      pointer operator->() const { return &Traits::value(m_p); }

      const_iterator& operator++() { m_p = next(m_p); return *this; }
      const_iterator operator++(int) { const_iterator c(m_p); ++(*this); return c; }
      bool operator==( const const_iterator& rhs ) const { return m_p == rhs.m_p; }
      bool operator!=( const const_iterator& rhs ) const { return m_p != rhs.m_p; }
  };

  iterator begin() { return m_size ? first(dictionary()->stored()) : NULL; }
  iterator end() { return iterator(); }

  const_iterator begin() const { return m_size ? first(dictionary()->stored()) : NULL; }
  const_iterator end() const { return const_iterator(); }

  inline const_iterator find(const key_type& key) const { return find_this(key); }
  inline iterator find(const key_type& key) { return find_this(key); }

  std::pair<iterator, bool> insert(const value_type& value) {
    if ( m_size >= m_watermark ) {
      rehash( (m_max_index + 1) << 1 );
    }

    stored_type* p = dictionary()->stored();
    std::size_t i, step = 0;
    hash_type h = typename Traits::hasher()(Traits::value_key(value));
    for ( i = h & m_max_index; !Traits::empty(p + i); i = (i + ++step) & m_max_index ) {
      if ( h == Traits::value_attr(p + i) &&
           key_equal()(Traits::value_key(value), Traits::value_key(Traits::value(p + i))) )
        return std::pair<iterator, bool>(iterator(p + i), false);
    }
    new (&Traits::value(p + i)) value_type(value);
    Traits::assign_attr(p + i, h, i == m_max_index);
    m_size++;
    return std::pair<iterator, bool>(iterator(p + i), true);
  }

  mapped_type& operator[](const key_type& key) {
    iterator it = find_this(key);
    if (it != end()) return it->second;
    return insert(value_type(key, mapped_type())).first->second;
  }

  bool empty() const { return m_size == 0; }

  std::size_t size() const { return m_size; }

  protected:

  DictionaryBase()
  : m_max_index(0), m_size(0), m_watermark(0) {}

  stored_type* HEAVYUSE find_this(const key_type& k) const {
    stored_type* p = dictionary()->stored();
    if ( p ) {
      std::size_t i, step = 0;
      hash_type h = typename Traits::hasher()(k);
      for ( i = h & m_max_index; !Traits::empty(p + i); i = (i + ++step) & m_max_index ) {
        if ( h == Traits::value_attr(p + i) &&
             key_equal()(k, Traits::value_key(Traits::value(p + i))) )
          return p + i;
      }
    }
    return NULL;
  }

  void destroy() {
    if ( m_size ) {
      stored_type* p = dictionary()->stored();
      for (iterator it = first(p); it != end(); ++it) 
        it->~value_type();
      dictionary()->release(p, m_max_index + 1);
    }
  }

  void dup( stored_type* src, std::size_t capacity, std::size_t size, bool move = false ) {
    stored_type* p = dictionary()->reserve(capacity);
    m_size = 0;
    m_max_index = capacity - 1;
    m_watermark = capacity > 8 ? capacity * MaxLoadPercentage / 100 : capacity * 3 / 4 ;
    Traits::format_attr(p, m_max_index);
    if ( size ) {
      if ( move ) {
        for ( iterator it = first(src); it != end(); ++it ) {
          insert(*it);
          it->~value_type();
        }
      } else
        for ( iterator it = first(src); it != end(); ++it )
          insert(*it);
    }
  }

  void dup( stored_type* src, const DictionaryBase& other ) {
    if ( src )
      dup( src, other.m_max_index + 1, other.m_size );
    else
      m_max_index = m_size = m_watermark = 0;
  }

  private:

  void rehash( std::size_t target_capacity ) {
    stored_type* p = dictionary()->stored();
    std::size_t index = m_max_index;
    if ( target_capacity > index ) {
      std::size_t size = m_size;
      this->dup(p, target_capacity, size, true);
      if ( p )
        dictionary()->release(p, index + 1);
    }
  }

  std::size_t m_max_index;
  std::size_t m_size;
  std::size_t m_watermark;
};

template <typename Key, typename Hash>
struct TraitsBase {
  typedef Key key_type;
  typedef std::size_t hash_type;

  static const int HashBits = (sizeof(hash_type) << 3) - 2; // reserve two upper bits for markers
  static const std::size_t HashMask = ((hash_type)1 << (HashBits - 1)) - 1;
  static const std::size_t Terminal = (hash_type)1 << HashBits;
  static const std::size_t Empty = ((hash_type)1 << (HashBits + 1)); // top bit

  static const std::size_t PadSz = 0;

  struct hasher {
    hash_type operator()(const key_type& key) { return Hash()(key) & HashMask; }
  };
};

// Specialization for tag as key (positive integer)
template <>
struct TraitsBase<int, Util::Tag::Identity> {
  typedef int key_type;
  typedef int hash_type;

  static const int Empty = ((hash_type)1 << ((sizeof(hash_type) << 3) - 1)); // top bit
  static const int Terminal = ~0; // includes Empty bit

  static const std::size_t PadSz = 1;

  struct hasher {
    hash_type operator()(const int& key) { return key; }; // max tag value limited to 99999999
  };
};

template <typename Key, typename Mapped, typename Hash, typename Pred>
struct MapTraits : public TraitsBase<Key, Hash> {

  typedef TraitsBase<Key, Hash> base_type;
  typedef typename base_type::key_type key_type;
  typedef typename base_type::hash_type hash_type;

  typedef Mapped mapped_type;
  typedef std::pair<key_type, mapped_type> value_type;
  typedef std::pair<hash_type, value_type> stored_type;
  typedef Pred key_equal;

  static inline bool empty(const stored_type* p) { return base_type::Empty & p->first; }
  static inline bool last(const stored_type* p) { return base_type::Terminal & p->first; }

  static inline hash_type   value_attr(const stored_type* p) { return p->first & base_type::HashMask; }
  static inline value_type& value(stored_type* p) { return p->second; }
  static inline const value_type& value(const stored_type* p) { return p->second; }
  static inline const key_type& value_key(const value_type& v) { return v.first; }

  static inline void assign_attr(stored_type* p, hash_type h, bool last) {
    p->first = h | (last ? base_type::Terminal : 0);
  }
  static inline void format_attr(stored_type* p, std::size_t max_index) {
    for ( std::size_t i = 0; i < max_index; i++) p++->first = base_type::Empty;
    p->first = base_type::Empty | base_type::Terminal;
  }
};

template <typename Mapped>
struct MapTraits<int, Mapped, Util::Tag::Identity, std::equal_to<int> >
: public TraitsBase<int, Util::Tag::Identity> {

  typedef TraitsBase<int, Util::Tag::Identity> base_type;
  typedef base_type::key_type key_type;
  typedef base_type::hash_type hash_type;

  typedef Mapped mapped_type;
  typedef std::pair<hash_type, mapped_type> value_type;
  typedef value_type stored_type;
  typedef std::equal_to<int> key_equal;

  static inline bool empty(const stored_type* p) { return base_type::Empty == p->first; }
  static inline bool last(const stored_type* p) { return base_type::Terminal == p[1].first; }

  static inline hash_type   value_attr(const stored_type* p) { return p->first; }
  static inline value_type& value(stored_type* p) { return *p; }
  static inline const value_type& value(const stored_type* p) { return *p; }
  static inline key_type    value_key(const value_type& v) { return v.first; }

  static inline void assign_attr(stored_type* p, hash_type h, bool) {
    p->first = h;
  }
  static inline void format_attr(stored_type* p, std::size_t max_index) {
    for ( std::size_t i = 0; i <= max_index; i++) p++->first = base_type::Empty;
    p->first = base_type::Terminal;
  }
};

template <typename Key, typename Mapped, typename Hash,
          typename Pred = std::equal_to<Key>, 
          typename Alloc = std::allocator<std::pair<Key const, Mapped> > >
class DictionaryMap
: public DictionaryBase<
  DictionaryMap<Key, Mapped, Hash, Pred, Alloc>,
  MapTraits< Key, Mapped, Hash, Pred >
>
{
  template <typename D, typename T> friend class DictionaryBase;
  typedef MapTraits< Key, Mapped, Hash, Pred > Traits;
  typedef DictionaryBase<
            DictionaryMap<Key, Mapped, Hash, Pred, Alloc>,
            MapTraits< Key, Mapped, Hash, Pred >
          > base_type;
  typedef typename base_type::stored_type stored_type;
  public:

  typedef Alloc allocator_type;

  DictionaryMap()
  : m_values(NULL), m_alloc(allocator_type())
  {}

  DictionaryMap(allocator_type a) 
  : m_values(NULL), m_alloc(a)
  {}
  
  DictionaryMap(const DictionaryMap& other)
  : m_values(NULL), m_alloc(other.m_alloc) {
    this->dup(other.m_values, other );
  }

  ~DictionaryMap() { this->destroy(); }

  DictionaryMap& operator=(const DictionaryMap& other) {
    this->destroy();
    m_values = NULL;
    this->dup(other.m_values, other );
    return *this;
  }

  protected:

  stored_type* stored() const { return m_values; }
  stored_type* reserve(std::size_t capacity) {
    return (m_values = m_alloc.allocate( capacity + Traits::PadSz ));
  }
  void release(stored_type* p, std::size_t capacity) {
    m_alloc.deallocate( p, capacity + Traits::PadSz );
  }

  private:

  typedef typename Alloc::template rebind<stored_type>::other stored_allocator_type;
  
  stored_type* m_values;
  stored_allocator_type m_alloc;
};

template <typename Key, typename Hash, typename Pred>
struct SetTraits : public TraitsBase<Key, Hash> {

  typedef TraitsBase<Key, Hash> base_type;
  typedef typename base_type::key_type key_type;
  typedef typename base_type::hash_type hash_type;

  typedef key_type mapped_type;
  typedef key_type value_type;
  typedef std::pair<hash_type, value_type> stored_type;
  typedef Pred key_equal;

  static inline bool empty(const stored_type* p) { return base_type::Empty & p->first; }
  static inline bool last(const stored_type* p) { return base_type::Terminal & p->first; }

  static inline hash_type   value_attr(const stored_type* p) { return p->first & base_type::HashMask; }
  static inline value_type& value(stored_type* p) { return p->second; }
  static inline const value_type& value(const stored_type* p) { return p->second; }
  static inline const key_type& value_key(const value_type& v) { return v; }

  static inline void assign_attr(stored_type* p, hash_type h, bool last) {
    p->first = h | (last ? base_type::Terminal : 0);
  }
  static inline void format_attr(stored_type* p, std::size_t max_index) {
    for ( std::size_t i = 0; i < max_index; i++) p++->first = base_type::Empty;
    p->first = base_type::Empty | base_type::Terminal;
  }
};

template <>
struct SetTraits<int, Util::Tag::Identity, std::equal_to<int> >
: public TraitsBase<int, Util::Tag::Identity> {

  typedef TraitsBase<int, Util::Tag::Identity> base_type;

  typedef key_type mapped_type;
  typedef key_type value_type;
  typedef hash_type stored_type;
  typedef std::equal_to<int> key_equal;

  static inline bool empty(const stored_type* p) { return Empty == *p; }
  static inline bool last(const stored_type* p) { return Terminal == p[1]; }

  static inline hash_type   value_attr(const stored_type* p) { return *p; }
  static inline value_type& value(stored_type* p) { return *p; }
  static inline const value_type& value(const stored_type* p) { return *p; }
  static inline key_type    value_key(const value_type& v) { return v; }

  static inline void assign_attr(stored_type* p, hash_type h, bool) {
    *p = h;
  }
  static inline void format_attr(stored_type* p, std::size_t max_index) {
    for ( std::size_t i = 0; i <= max_index; i++) *p++ = base_type::Empty;
    *p = base_type::Terminal;
  }
};

template <typename Key, typename Hash,
          typename Pred = std::equal_to<Key>, 
          typename Alloc = std::allocator<Key> >
class DictionarySet
: public DictionaryBase<
  DictionarySet< Key, Hash, Pred, Alloc >,
  SetTraits< Key, Hash, Pred >
>
{
  template <typename D, typename T> friend class DictionaryBase;
  typedef SetTraits< Key, Hash, Pred > Traits;
  typedef DictionaryBase<
            DictionarySet< Key, Hash, Pred, Alloc >,
            SetTraits< Key, Hash, Pred >
          > base_type;
  typedef typename base_type::stored_type stored_type;
  public:

  typedef Alloc allocator_type;

  DictionarySet()
  : m_values(NULL), m_alloc(allocator_type())
  {}

  DictionarySet(allocator_type a) 
  : m_values(NULL), m_alloc(a)
  {}
  
  DictionarySet(const DictionarySet& other)
  : m_values(NULL), m_alloc(other.m_alloc) {
    this->dup(other.m_values, other );
  }

  ~DictionarySet() { this->destroy(); }

  DictionarySet& operator=(const DictionarySet& other) {
    this->destroy();
    m_values = NULL;
    this->dup(other.m_values, other );
    return *this;
  }

  protected:

  stored_type* stored() const { return m_values; }
  stored_type* reserve(std::size_t capacity) {
    return (m_values = m_alloc.allocate( capacity + Traits::PadSz ));
  }
  void release(stored_type* p, std::size_t capacity) {
    m_alloc.deallocate( p, capacity + Traits::PadSz );
  }

  private:

  typedef typename Alloc::template rebind<stored_type>::other stored_allocator_type;
  
  stored_type* m_values;
  stored_allocator_type m_alloc;
};

#ifdef HAVE_ULIB
  struct ulib_hash_base {
    template <typename T, typename Hash>
    struct Key {
      struct type {
      T m_key;
      type(const Key& other) : m_key(other.m_key) {}
      type& operator=(const Key& other) { m_key = other.m_key; }
      type(const T& k) : m_key(k) {}
      bool operator==(const type& other) const { return m_key == other.m_key; }
      bool operator<(const type& other) const { return m_key < other.m_key; }
      operator unsigned long() const { return Hash()(m_key); }
      type& operator=(const T& k) { m_key = k; return *this; }
      };
    };
  };

  template<typename Hash> struct ulib_hash_base::Key<int, Hash> {
    typedef int type;
  };
  template <typename T, typename Hash>
  class ulib_hash_set : public ulib_hash_base {
    typedef typename Key<T, Hash>::type key_type;
    typedef ulib::open_hash_map<key_type, bool> type;
    public:
    typedef typename type::iterator iterator;
    typedef typename type::const_iterator const_iterator;
    void insert( const T value) { m_data.insert(value, true); }
    const_iterator find( const T& value ) const { return m_data.find( value ); }
    const_iterator end() const { return m_data.end(); }
    private:
    type m_data;
  };

  template <typename T, typename V, typename Hash,
            typename Alloc = std::allocator< std::pair<T, V> > >
  class ulib_hash_map : public ulib_hash_base {
    typedef typename Key<T, Hash>::type key_type;
    typedef std::pair<T, V> stored_type;
    typedef stored_type* stored_type_ptr;
    typedef ulib::open_hash_map<key_type, stored_type_ptr > type;
    typedef typename type::iterator _iterator;
    typedef typename type::const_iterator _const_iterator;
    typedef typename Alloc::template rebind<stored_type>::other stored_data_allocator_type;
    public:
    typedef stored_type value_type;
    typedef typename Alloc::template rebind<value_type>::other allocator_type;
    typedef V& reference;
    class iterator {
      _iterator m_it;
      public:
      typedef value_type& reference;
      typedef value_type* pointer;
      iterator(const _iterator& it) : m_it(it) {}
      reference operator*() { return **m_it; }
      pointer operator->() { return *m_it; }
      iterator& operator++() { ++m_it; return *this; }
      bool operator==(const iterator& other) { return m_it == other.m_it; }
      bool operator!=(const iterator& other) { return m_it != other.m_it; }
    };
    class const_iterator {
      _const_iterator m_it;
      public:
      typedef const value_type& reference;
      typedef const value_type* pointer;
      const_iterator(const _const_iterator& it) : m_it(it) {}
      reference operator*() const { return **m_it; }
      pointer operator->() const { return  *m_it; }
      const_iterator& operator++() { ++m_it; return *this; }
      bool operator==(const const_iterator& other) { return m_it == other.m_it; }
      bool operator!=(const const_iterator& other) { return m_it != other.m_it; }
    };

    ulib_hash_map(allocator_type a) : m_allocator(a) {}
    ~ulib_hash_map() {
      for (_iterator i = m_data.begin(); i != m_data.end(); ++i) {
        i.value()->~stored_type();
        m_allocator.deallocate(i.value(), sizeof(stored_type));
      }
    }

    std::pair<iterator, bool> insert( const value_type& v) {
      _iterator it = m_data.insert(v.first, NULL);
      if (!it.value()) {
        it.value() = new (m_allocator.allocate(sizeof(stored_type))) stored_type(v.first, v.second);
        return std::pair<iterator, bool>(iterator(it), true);
      }
      return std::pair<iterator, bool>(iterator(it), false);
    }

    reference operator[](const T& key ) {
      _iterator it = m_data.find_or_insert(key, NULL);
      if (!it.value())
        it.value() = new (m_allocator.allocate(sizeof(stored_type))) stored_type(key, V());
      return it.value()->second;
    }

    const_iterator find( const T& key ) const { return m_data.find( key ); }
    const_iterator begin() const { return m_data.begin(); }
    const_iterator end() const { return m_data.end(); }
    iterator find( const T& key ) { return m_data.find( key ); }
    iterator begin() { return m_data.begin(); }
    iterator end() { return m_data.end(); }
    private:
    stored_data_allocator_type m_allocator;
    type m_data;
  };
#endif // ULIB

#ifdef HAVE_RDESTL
  template <typename A>
  class rde_fwd_allocator
  {
    typedef typename A::template rebind<char>::other allocator;
    allocator m_allocator;
    public:
    explicit rde_fwd_allocator() {}
    explicit rde_fwd_allocator(A a) : m_allocator(a) {}
    ~rde_fwd_allocator() {}

    void* allocate(unsigned int bytes, int flags = 0)
    { return m_allocator.allocate(bytes); }
    void deallocate(void* ptr, unsigned int bytes)
    { m_allocator.deallocate((char*)ptr, bytes); }
  };

  template <typename Key, typename Hash, typename Pred, typename Alloc = std::allocator<Key> >
  struct rde_hash_set
  {
    typedef rde::hash_map <Key, bool, Hash, 6, Pred, rde_fwd_allocator<Alloc> > map_type;
    class type {
      public:
      typedef typename map_type::allocator_type allocator_type;
      typedef typename map_type::value_type value_type;
      typedef typename map_type::iterator iterator;
      typedef typename map_type::const_iterator const_iterator;
      type() {}
      type(allocator_type a) : m_allocator(a), m_data(a) {}
      type(const type& src) : m_allocator(src.m_allocator), m_data(src.m_data, src.m_allocator) {}
      type(const type& src, typename type::allocator_type a) : m_allocator(a), m_data(src.m_data, a) {}
      rde::pair<iterator, bool> insert( const Key& value ) { return m_data.insert( rde::make_pair(value, true) ); }
      const_iterator find( const Key& value ) const { return m_data.find(value); }
      const_iterator end() const { return m_data.end(); }
      private:
      allocator_type m_allocator;
      map_type m_data;
    };
  };

  template <typename Key, typename Mapped, typename Hash, typename Pred,
    typename Alloc = std::allocator< std::pair< Key, Mapped > > >
  struct rde_hash_map {
    typedef rde::hash_map < Key, Mapped, Hash, 6, Pred, rde_fwd_allocator<Alloc> > type;
  };

  template <typename T, typename Alloc = std::allocator< T > >
  struct rde_vector {
    typedef rde::vector < T, rde_fwd_allocator<Alloc> > type;
  };
#endif //RDESTL


} // Container
}

#endif //FIX_CONTAINER_H

