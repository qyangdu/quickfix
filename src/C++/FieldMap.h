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

#ifndef FIX_FIELDMAP
#define FIX_FIELDMAP

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "Field.h"
#include "MessageSorters.h"
#include "Exceptions.h"
#include "ItemAllocator.h"
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

#if defined(ENABLE_BOOST_MAP)
#include <boost/container/map.hpp>
#elif defined(ENABLE_BOOST_RBTREE)
#include <boost/intrusive/rbtree.hpp>
#elif defined(ENABLE_BOOST_SGTREE)
#include <boost/intrusive/sgtree.hpp>
#elif defined(ENABLE_BOOST_AVLTREE)
#include <boost/intrusive/avltree.hpp>
#endif

namespace FIX
{
/**
 * Stores and organizes a collection of Fields.
 *
 * This is the basis for a message, header, and trailer.  This collection
 * class uses a sorter to keep the fields in a particular order.
 */
class FieldMap
{
#if defined(ENABLE_BOOST_RBTREE) || defined(ENABLE_BOOST_SGTREE) || defined(ENABLE_BOOST_AVLTREE)

#if defined(ENABLE_BOOST_RBTREE)
  typedef boost::intrusive::set_member_hook<
#elif defined(ENABLE_BOOST_SGTREE)
  typedef boost::intrusive::bs_set_member_hook<
#elif defined(ENABLE_BOOST_AVLTREE)
  typedef boost::intrusive::avl_set_member_hook<
#endif
    boost::intrusive::link_mode<boost::intrusive::normal_link>,
    boost::intrusive::void_pointer<void*>,
    boost::intrusive::optimize_size<false>
  > store_hook;

  class stored_type {
    public:

    store_hook m_link;
#ifdef ENABLE_SLIST_TREE_TRAVERSAL
    mutable const stored_type* m_next;
#endif
    int first;
    FieldBase second;

    struct compare_type : public message_order::comparator {
      typedef message_order::comparator base_type;
      compare_type( const message_order& order ) : base_type( order ) {}
      bool operator()(const stored_type& a, const stored_type& b) const
      { return base_type::operator()(a.first, b.first); }
      bool operator()(int a, const stored_type& b) const 
      { return base_type::operator()(a, b.first); }
      bool operator()(const stored_type& a, int b) const 
      { return base_type::operator()(a.first, b); }
    };

    template <typename A> struct disposer_type {
      A& m_allocator;
      disposer_type(A& a) : m_allocator(a) {}
      void operator()(stored_type* p) {
        p->~stored_type();
        m_allocator.deallocate(p, 1);
      }
    };

    template <typename A> struct cloner_type {
      A& m_allocator;
      cloner_type(A& a) : m_allocator(a) {}
      stored_type* operator()(const stored_type& r) {
        return new (m_allocator.allocate(1)) stored_type(r);
      }
    };

    template <typename Arg> stored_type( const Arg& arg ) : first(arg.getField()), second( arg ) {}
    template <typename Arg> stored_type( int tag, const Arg& arg ) : first(tag), second( arg ) {}
    stored_type( int tag, const std::string& value ) : first(tag), second( tag, value ) {}
  };

  typedef boost::intrusive::member_hook< stored_type, store_hook, &stored_type::m_link > hook_type;
  typedef boost::intrusive::compare<stored_type::compare_type> compare_type;
#if defined(ENABLE_BOOST_RBTREE)
  typedef boost::intrusive::rbtree<
#elif defined(ENABLE_BOOST_SGTREE)
  typedef boost::intrusive::sgtree<
#elif defined(ENABLE_BOOST_AVLTREE)
  typedef boost::intrusive::avltree<
#endif
    stored_type,
    compare_type,
    hook_type,
    boost::intrusive::constant_time_size<false>
  > store_type;

  static const std::size_t AllocationUnit = sizeof(stored_type);
  ItemAllocator< stored_type > m_allocator;

  public:

  typedef ItemAllocator< stored_type > allocator_type;
  allocator_type get_allocator() { return m_allocator; }

#else //!ENABLE_BOOST_xxTREE
#ifdef ENABLE_BOOST_MAP
  typedef boost::container::multimap < int, FieldBase, message_order::comparator,
                          ItemAllocator < std::pair<const int, FieldBase> > > store_type;
  typedef store_type::const_iterator field_iterator;

  static const std::size_t AllocationUnit = sizeof(store_type::stored_allocator_type::value_type);
#else
  typedef std::multimap < int, FieldBase, message_order::comparator,
                          ItemAllocator < std::pair<const int, FieldBase> > > store_type;
  typedef store_type::const_iterator field_iterator;

  static const std::size_t AllocationUnit;
  static std::size_t init_allocation_unit();
#endif

  public:

  typedef store_type::allocator_type allocator_type;
  allocator_type get_allocator()
  { return m_fields.get_allocator(); }

#endif //!ENABLE_BOOST_xxTREE

  private:

  typedef std::vector < FieldMap *, ALLOCATOR < FieldMap* > > GroupItem;
  typedef std::map < int, GroupItem, std::less<int>, 
                          ALLOCATOR < std::pair<const int, GroupItem> > > Groups;

  void g_clear()
  {
    Groups::const_iterator i, end = m_groups.end();
    for ( i = m_groups.begin(); i != end; ++i )
    {
      GroupItem::const_iterator j, jend = i->second.end();
      for ( j = i->second.begin(); j != jend; ++j )
        delete *j;
    }
    m_groups.clear();
  }

  void g_copy(const FieldMap& src)
  {
    Groups::const_iterator i, end = src.m_groups.end();
    for ( i = src.m_groups.begin(); i != end; ++i )
    {
      GroupItem::const_iterator j, jend = i->second.end();
      for ( j = i->second.begin(); j != jend; ++j )
      {
        FieldMap * pGroup = new FieldMap( FieldMap::create_allocator(), **j );
        m_groups[ i->first ].push_back( pGroup );
      }
    }
  }

  template <typename Arg> static inline void assign_value(store_type::iterator& it, const Arg& arg)
  { it->second.setPacked(arg); }
  void assign_value(FieldMap::store_type::iterator& it, const FieldBase& field)
  { it->second = field; }
  void assign_value(FieldMap::store_type::iterator& it, const std::string& value)
  { it->second.setString( value ); }

#if defined(ENABLE_BOOST_RBTREE) || defined(ENABLE_BOOST_SGTREE) || defined(ENABLE_BOOST_AVLTREE)

#ifdef ENABLE_SLIST_TREE_TRAVERSAL
  class field_iterator {

    public:
    typedef const stored_type* const_pointer;
    typedef stored_type* pointer;
    typedef const stored_type& const_reference;
    typedef stored_type& reference;
    field_iterator(const stored_type* p = NULL) : m_p(p) {}
    field_iterator& operator ++() { m_p = m_p->m_next; return *this; }
    field_iterator  operator ++(int) { const stored_type* p = m_p; m_p = p->m_next; return p; }
    bool operator == (const field_iterator& other) const { return m_p == other.m_p; }
    bool operator != (const field_iterator& other) const { return m_p != other.m_p; }
    const_reference operator*() { return *m_p; }
    const_pointer operator->() { return m_p; }

    private:
    const stored_type* m_p;
  };

  struct stored_type_slist {
    const stored_type* m_root;
    const stored_type* m_last;

    stored_type_slist() : m_root(NULL), m_last(NULL) {}
    void clear() { m_root = m_last = NULL; }
    bool empty() const { return m_root == NULL; }
    void push_back(const stored_type* p) {
      if (LIKELY(NULL != m_last)) {
        m_last->m_next = p;
        m_last = p;
      } else
        m_root = m_last = p;
      p->m_next = NULL;
    }

    struct cloner {
      stored_type_slist& m_s;
      const stored_type** m_p;

      cloner(stored_type_slist& s) : m_s(s), m_p(&s.m_root) {}
      void next(const stored_type* p) { *m_p = p; m_p = &p->m_next; }
      void final(const stored_type* p) { *m_p = NULL; m_s.m_last = p; }
    };
  } m_list;

  inline void HEAVYUSE f_clear() {
    m_fields.clear_and_dispose(stored_type::disposer_type<allocator_type>(m_allocator));
    m_list.clear();
  }

  inline void f_copy(const FieldMap& from) {
    stored_type *n = NULL;
    stored_type_slist::cloner c( m_list );
    const store_type& src = from.m_fields;
    store_type::const_iterator e = src.end();
    for ( store_type::const_iterator it = src.begin(); it != e; ++it ) {
      n = new ( m_allocator.allocate(1) ) stored_type(*it);
      m_fields.push_back( *n );
      c.next( n );
    }
    c.final( n );
  }

  inline void f_clone(const FieldMap& from) {
    m_fields.clone_from( from.m_fields, stored_type::cloner_type<allocator_type>(m_allocator),
                                        stored_type::disposer_type<allocator_type>(m_allocator));
    m_order = from.m_order;

    stored_type *n = NULL;
    stored_type_slist::cloner c( m_list );
    store_type::iterator e = m_fields.end();
    for ( store_type::iterator it = m_fields.begin(); it != e; ++it ) {
      n = &*it;
      c.next( n );
    }
    c.final( n );
  }

  inline field_iterator f_begin() const { return field_iterator(m_list.m_root); }
  static inline field_iterator f_end() { return field_iterator(); }

  void unlink_next(const stored_type* removed, store_type::const_iterator next) {
    if ( !m_fields.empty() ) {
      if ( m_list.m_last != removed ) {
        if ( m_list.m_root != removed )
        { removed = &*next; (--next)->m_next = removed; }
        else
        { m_list.m_root = &*next; }
      } else { (m_list.m_last = &*--next)->m_next = NULL; }
    } else
      m_list.clear();
  }

  inline store_type::const_iterator to_store_iterator(field_iterator it) {
    return it == field_iterator() ? m_fields.end() : m_fields.iterator_to(*it);
  }

  inline field_iterator HEAVYUSE link_next(store_type::const_iterator inserted) {
    const stored_type* p = &*inserted;
    store_type::const_iterator it = inserted;
    if ( ++it == m_fields.end() )
      m_list.push_back(p);
    else {
      p->m_next = &*it;
      if ( inserted == m_fields.begin() ) 
        m_list.m_root = p;
      else {
        (--inserted)->m_next = p;
      }
    }
    return p;
  }

  inline void erase(int tag) {
    store_type::iterator it = m_fields.find( tag, m_fields.value_comp() );
    if ( it != m_fields.end() ) {
      const stored_type* p = &*it;
      it = m_fields.erase_and_dispose(it, stored_type::disposer_type<allocator_type>(m_allocator));
      unlink_next(p, it);
    }
  }
  inline store_type::iterator erase(store_type::iterator& it) {
    const stored_type* p = &*it;
    it = m_fields.erase_and_dispose( it, stored_type::disposer_type<allocator_type>(m_allocator));
    unlink_next(p, it);
    return it;
  }

  template <typename Arg> stored_type& HEAVYUSE push_back(const Arg& arg) {
    stored_type* p = new (m_allocator.allocate(1)) stored_type( arg );
    m_list.push_back(p);
    m_fields.push_back(*p);
    return *p;
  }
#else //!ENABLE_SLIST_TREE_TRAVERSAL

  typedef store_type::const_iterator field_iterator;

  inline void HEAVYUSE f_clear() {
    m_fields.clear_and_dispose(stored_type::disposer_type<allocator_type>(m_allocator));
  }

  inline void f_copy(const FieldMap& from) {
    const store_type& src = from.m_fields;
    store_type::const_iterator e = src.end();
    for (store_type::const_iterator it = src.begin(); it != e; ++it)
      m_fields.push_back( *new (m_allocator.allocate(1)) stored_type(*it) );
  }

  inline void f_clone(const FieldMap& from) {
    m_fields.clone_from( from.m_fields, stored_type::cloner_type<allocator_type>(m_allocator),
                                        stored_type::disposer_type<allocator_type>(m_allocator));
    m_order = from.m_order;
  }

  inline field_iterator f_begin() const { return m_fields.begin(); }
  inline field_iterator f_end() const { return m_fields.end(); }

  static inline store_type::const_iterator to_store_iterator(const field_iterator it) { return it; }
  static inline field_iterator link_next(const store_type::const_iterator it) { return it; }

  inline void erase(int tag) {
    store_type::iterator it = m_fields.find( tag, m_fields.value_comp() );
    if ( it != m_fields.end() ) m_fields.erase_and_dispose(it, stored_type::disposer_type<allocator_type>(m_allocator));
  }
  inline store_type::iterator erase(store_type::iterator& it) {
    return m_fields.erase_and_dispose( it, stored_type::disposer_type<allocator_type>(m_allocator));
  }

  template <typename Arg> stored_type& HEAVYUSE push_back(const Arg& arg) {
    stored_type* p = new (m_allocator.allocate(1)) stored_type( arg );
    m_fields.push_back(*p);
    return *p;
  }
#endif //!ENABLE_SLIST_TREE_TRAVERSAL

  inline store_type::iterator HEAVYUSE lower_bound(int tag) {
    return m_fields.lower_bound( tag, m_fields.value_comp() );
  }
  inline store_type::const_iterator HEAVYUSE find(int tag) const {
    return m_fields.find( tag, m_fields.value_comp() );
  }

  template <typename Arg> field_iterator HEAVYUSE add(const Arg& arg) {
    return link_next(m_fields.insert_equal(*new (m_allocator.allocate(1)) stored_type( arg )));
  }
  template <typename Arg> field_iterator HEAVYUSE add(field_iterator hint, const Arg& arg) {
    return link_next(m_fields.insert_equal(to_store_iterator(hint), *new (m_allocator.allocate(1)) stored_type( arg )));
  }

  template <typename Arg> store_type::iterator HEAVYUSE assign(int tag, const Arg& arg) {
    store_type::insert_commit_data data;
    std::pair<store_type::iterator, bool> r = m_fields.insert_unique_check( tag, m_fields.value_comp(), data);
    if (r.second) link_next(r.first = m_fields.insert_unique_commit( *new (m_allocator.allocate(1)) stored_type( tag, arg ), data ));
    else assign_value(r.first, arg );
    return r.first;
  }

#else // !ENABLE_BOOST_xxTREE

  inline void HEAVYUSE f_clear() {
    m_fields.clear();
  }

  inline void f_copy(const FieldMap& from) {
    const store_type& src = from.m_fields;
    store_type::const_iterator e = src.end();
    for (store_type::const_iterator it = src.begin();  it != e; ++it)
      m_fields.insert(m_fields.end(), *it);
  }

  inline void f_clone(const FieldMap& from) {
    m_fields = from.m_fields;
    m_order = from.m_order;
  }

  inline field_iterator f_begin() const { return m_fields.begin(); }
  inline field_iterator f_end() const { return m_fields.end(); }

  inline void erase(int tag) {
    store_type::iterator it = m_fields.find(tag);
    if ( it != m_fields.end() ) m_fields.erase(it);
  }
  inline store_type::iterator erase(store_type::iterator& it) {
    m_fields.erase(it++);
    return it;
  }

  inline store_type::iterator HEAVYUSE lower_bound(int tag) {
    return m_fields.lower_bound( tag );
  }
  inline store_type::const_iterator HEAVYUSE find(int tag) const {
    return m_fields.find( tag );
  }

#ifdef ENABLE_BOOST_MAP
  template <typename Arg> store_type::value_type& HEAVYUSE push_back(const Arg& arg) {
    return *m_fields.emplace( m_fields.end(), arg.getField(), arg );
  }
  template <typename Arg> store_type::const_iterator HEAVYUSE add(const Arg& arg) {
    return m_fields.emplace( arg.getField(), arg );
  }
  template <typename Arg> store_type::const_iterator HEAVYUSE add(store_type::const_iterator hint, const Arg& arg) {
    return m_fields.emplace_hint( hint, arg.getField(), arg );
  }

  template <typename Arg> store_type::iterator HEAVYUSE assign(int tag, const Arg& arg) {
    store_type::iterator it = m_fields.lower_bound( tag );
    if ( it == m_fields.end() || it->first != tag )
      return m_fields.emplace_hint( it, tag, arg );
    assign_value( it, arg );
    return it;
  }
  store_type::iterator assign(int tag, const std::string& value) {
    store_type::iterator it = m_fields.lower_bound( tag );
    if ( it == m_fields.end() || it->first != tag )
      it = m_fields.emplace_hint( it, tag, tag );
    assign_value( it, value );
    return it;
  }
#else // !ENABLE_BOOST_MAP
  template <typename Arg> store_type::value_type& HEAVYUSE push_back(const Arg& arg) {
    store_type::iterator it = m_fields.insert( m_fields.end(), store_type::value_type( arg.getField(), FieldBase( arg.getField() ) ) );
    assign_value( it, arg );
    return *it;
  }
  template <typename Arg> store_type::const_iterator HEAVYUSE add(const Arg& arg) {
    store_type::iterator it = m_fields.insert( store_type::value_type( arg.getField(), FieldBase( arg.getField() ) ) );
    assign_value( it, arg );
    return it;
  }
  template <typename Arg> store_type::const_iterator HEAVYUSE add(store_type::const_iterator hint, const Arg& arg) {
    store_type::iterator it = m_fields.insert( hint, store_type::value_type( arg.getField(), FieldBase( arg.getField() ) ) );
    assign_value( it, arg );
    return it;
  }

  template <typename Arg> store_type::iterator HEAVYUSE assign(int tag, const Arg& arg) {
    store_type::iterator it = m_fields.lower_bound( tag );
    if ( it == m_fields.end() || it->first != tag )
      it = m_fields.insert( it, store_type::value_type( tag, FieldBase( tag ) ) );
    assign_value( it, arg );
    return it;
  }
#endif // !ENABLE_BOOST_MAP
#endif // !ENABLE_BOOST_xxTREE

protected:

  struct Sequence {
    /// Adds a field without type checking to the end of the map without order checking
    template <typename Packed>
    static inline store_type::value_type* push_back_to( FieldMap& map, const Packed& packed )
    { return &map.push_back( packed ); }
    static inline bool header_compare( const FieldMap&, int x, int y )
    { return message_order::header_compare( x, y ); }
    static inline bool trailer_compare( const FieldMap&, int x, int y )
    { return message_order::trailer_compare( x, y ); }
    static inline bool group_compare( const FieldMap& map, int x, int y )
    { return map.m_order.group_compare( x, y ); }
  };

public:

  typedef store_type Fields;

  typedef field_iterator         iterator;
  typedef field_iterator         const_iterator;

  typedef Groups::const_iterator g_iterator;
  typedef g_iterator             g_const_iterator;
  typedef Groups::mapped_type::const_iterator g_item_iterator;
  typedef g_item_iterator        g_item_const_iterator;

  FieldMap( const message_order& order =
            message_order( message_order::normal ) )
  : m_order( order ), m_fields( order ) {}

  FieldMap( const int order[] )
  : m_order( order ), m_fields( m_order ) {}

#if defined(ENABLE_BOOST_RBTREE) || defined(ENABLE_BOOST_SGTREE) || defined(ENABLE_BOOST_AVLTREE)
  FieldMap( const FieldMap& src )
  : m_order( src.m_order ), m_fields( src.m_fields.value_comp() )
  { f_copy(src); g_copy(src); }

  FieldMap( const allocator_type& a, const message_order& order =
            message_order( message_order::normal ) )
  : m_allocator( a ), m_order( order ), m_fields( order ) {}

  FieldMap( const allocator_type& a, const FieldMap& src )
  : m_allocator( a ), m_order( src.m_order ), m_fields( src.m_fields.value_comp() )
  { f_copy(src); g_copy(src); }

#else
  FieldMap( const FieldMap& src )
  : m_order( src.m_order ), m_fields( src.m_fields.key_comp() )
  { f_copy(src); g_copy(src); }

  FieldMap( const allocator_type& a, const message_order& order =
            message_order( message_order::normal ) )
  : m_order( order ), m_fields( order, a ) {}

  FieldMap( const allocator_type& a, const FieldMap& src )
  : m_order( src.m_order ), m_fields( src.m_fields.key_comp(), a )
  { f_copy(src); g_copy(src); }
#endif

  virtual ~FieldMap();

  FieldMap& operator=( const FieldMap& rhs );

  /// Adds a field without type checking by constructing in place
  template <typename Packed>
  iterator addField( const Packed& packed,
                     typename Packed::result_type* = NULL )
  { return add( end(), packed ); }

  iterator addField( const FieldBase& field )
  { return add( end(), field ); }

  template <typename Packed>
  iterator addField( FieldMap::iterator hint, const Packed& packed,
                     typename Packed::result_type* = NULL )
  { return add( hint, packed ); }

  iterator addField( FieldMap::iterator hint, const FieldBase& field )
  { return add( hint, field ); }

  /// Overwrite a field without type checking by constructing in place
  template <typename Packed>
  FieldBase& setField( const Packed& packed,
                       typename Packed::result_type* = NULL)
  { return assign( packed.getField(), packed )->second; }

  /// Set a field without type checking
  void setField( const FieldBase& field, bool overwrite = true )
  throw( RepeatedTag )
  {
    if ( overwrite )
      assign( field.getTag(), field );
    else
      add( field );
  }

  /// Set a field without a field class
  void setField( int tag, const std::string& value )
  throw( RepeatedTag, NoTagValue )
  { assign( tag, value ); }

  /// Get a field if set
  bool getFieldIfSet( FieldBase& field ) const
  {
    Fields::const_iterator iter = find( field.getTag() );
    if ( iter == m_fields.end() )
      return false;
    field = iter->second;
    return true;
  }

  /// Get a field without type checking
  FieldBase& getField( FieldBase& field )
  const throw( FieldNotFound )
  {
    Fields::const_iterator iter = find( field.getTag() );
    if ( iter == m_fields.end() )
      throw FieldNotFound( field.getTag() );
    field = iter->second;
    return field;
  }

  /// Get a field without a field class
  const std::string& getField( int tag )
  const throw( FieldNotFound )
  { return getFieldRef( tag ).getString(); }

  /// Get direct access to a field through a reference
  const FieldBase& getFieldRef( int tag )
  const throw( FieldNotFound )
  {
    Fields::const_iterator iter = find( tag );
    if ( iter == m_fields.end() )
      throw FieldNotFound( tag );
    return iter->second;
  }

  /// Get direct access to a field through a pointer
  const FieldBase* getFieldPtr( int tag )
  const throw( FieldNotFound )
  { return &getFieldRef( tag ); }

  /// Get direct access to a field through a pointer
  const FieldBase* getFieldPtrIfSet( int tag ) const
  {
    Fields::const_iterator iter = find( tag );
    return ( iter != m_fields.end() ) ? &iter->second : NULL;
  }

  /// Check to see if a field is set
  bool isSetField( const FieldBase& field ) const
  { return find( field.getTag() ) != m_fields.end(); }
  /// Check to see if a field is set by referencing its number
  bool isSetField( int tag ) const
  { return find( tag ) != m_fields.end(); }

  /// Remove a field. If field is not present, this is a no-op.
  void removeField( int tag )
  { erase( tag ); }

  /// Remove a range of fields 
  template <typename TagIterator>
  void removeFields(TagIterator tb, TagIterator te)
  {
    if( tb < te )
    {
      int tag, last = *(tb + ((te - tb) - 1));
      store_type::iterator end = m_fields.end();
      store_type::iterator it = lower_bound( *tb);
      while( it != end && (tag = it->first) <= last )
      {
        for( ; *tb < tag; ++tb )
          ;
        if( tag == *tb )
          it = erase( it );
        else
          ++it;
      }
    }
  }

  /// Add a group and return a reference to the added object.
  FieldMap& addGroup( int tag, const FieldMap& group, bool setCount = true );

  /// Acquire ownership of Group object
  void addGroupPtr( int tag, FieldMap* group, bool setCount = true );

  /// Replace a specific instance of a group.
  bool replaceGroup( int num, int tag, const FieldMap& group );

  /// Get a specific instance of a group.
  FieldMap& getGroup( int num, int tag, FieldMap& group ) const
  throw( FieldNotFound )
  { return group = getGroupRef( num, tag ); }

  /// Get direct access to a field through a reference
  FieldMap& getGroupRef( int num, int tag ) const
  throw( FieldNotFound )
  {
    Groups::const_iterator i = m_groups.find( tag );
    if( i == m_groups.end() ) throw FieldNotFound( tag );
    if( num <= 0 ) throw FieldNotFound( tag );
    if( i->second.size() < (unsigned)num ) throw FieldNotFound( tag );
    return *( *(i->second.begin() + (num-1) ) );
  }

  /// Get direct access to a field through a pointer
  FieldMap* getGroupPtr( int num, int tag ) const
  throw( FieldNotFound )
  { return &getGroupRef( num, tag ); }

  /// Remove a specific instance of a group.
  void removeGroup( int num, int tag );
  /// Remove all instances of a group.
  void removeGroup( int tag );

  /// Check to see any instance of a group exists
  bool hasGroup( int tag ) const;
  /// Check to see if a specific instance of a group exists
  bool hasGroup( int num, int tag ) const;
  /// Count the number of instance of a group
  int groupCount( int tag ) const;

  /// Clear all fields from the map
  void clear()
  {
    f_clear();
    g_clear();
  }

  /// Check if map contains any fields
  bool isEmpty()
  { return m_fields.empty(); }

  int totalFields() const;

  std::string& calculateString( std::string&, bool clear = true ) const;

  int calculateLength( int beginStringField = FIELD::BeginString,
                       int bodyLengthField = FIELD::BodyLength,
                       int checkSumField = FIELD::CheckSum ) const;

  int calculateTotal( int checkSumField = FIELD::CheckSum ) const;

  iterator begin() const { return f_begin(); }
  iterator end() const { return f_end(); }
  g_iterator g_begin() const { return m_groups.begin(); }
  g_iterator g_end() const { return m_groups.end(); }

  template <typename S> S& serializeTo( S& sink ) const
  {
    iterator i;
    for ( i = begin(); i != end(); ++i )
    {
      i->second.pushValue(sink);

      if( m_groups.empty() ) continue;
      Groups::const_iterator j = m_groups.find( i->first );
      if ( j == m_groups.end() ) continue;
      GroupItem::const_iterator ke = j->second.end();
      for ( GroupItem::const_iterator k = j->second.begin(); k != ke; ++k )
        ( *k ) ->serializeTo( sink );
    }
    return sink;
  }

  static inline
  allocator_type create_allocator(std::size_t n = ItemStore::MaxCapacity)
  {
    return allocator_type( ItemStore::buffer(n * AllocationUnit) );
  } 

protected:
  message_order m_order; // must precede field container
private:
  store_type m_fields;
  Groups m_groups;
};
/*! @} */
}

#define FIELD_SET( MAP, FIELD )           \
bool isSet( const FIELD& field ) const    \
{ return (MAP).isSetField(field); }       \
void set( const FIELD& field )            \
{ (MAP).setField(field); }                \
void set( const FIELD::Pack& packed )     \
{ (MAP).setField(packed); }               \
FIELD& get( FIELD& field ) const          \
{ return (FIELD&)(MAP).getField(field); } \
bool getIfSet( FIELD& field ) const       \
{ return (MAP).getFieldIfSet(field); }

#define FIELD_GET_PTR( MAP, FLD ) \
(const FIX::FLD*)MAP.getFieldPtr( FIX::FIELD::FLD )
#define FIELD_GET_REF( MAP, FLD ) \
(const FIX::FLD&)MAP.getFieldRef( FIX::FIELD::FLD )
#define FIELD_THROW_IF_NOT_FOUND( MAP, FLD ) \
if( !(MAP).isSetField( FIX::FIELD::FLD) ) \
  throw FieldNotFound( FIX::FIELD::FLD )

#endif //FIX_FIELDMAP

