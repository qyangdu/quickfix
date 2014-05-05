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

#ifdef HAVE_BOOST
#include <boost/container/map.hpp>
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
#ifdef HAVE_BOOST
  typedef boost::container::multimap < int, FieldBase, message_order::comparator, 
                          ItemAllocator < std::pair<const int, FieldBase> > > Fields;
  static const std::size_t AllocationUnit = sizeof(Fields::stored_allocator_type::value_type);
#else
  typedef std::multimap < int, FieldBase, message_order::comparator, 
                          ItemAllocator < std::pair<const int, FieldBase> > > Fields;
  static const std::size_t AllocationUnit;
  static std::size_t init_allocation_unit();
#endif
  typedef std::vector < FieldMap *, ALLOCATOR < FieldMap* > > GroupItem;
  typedef std::map < int, GroupItem, std::less<int>, 
                          ALLOCATOR < std::pair<const int, GroupItem> > > Groups;
public:

  typedef Fields::allocator_type allocator_type;

  typedef Fields::const_iterator iterator;
  typedef iterator               const_iterator;
  typedef Groups::const_iterator g_iterator;
  typedef g_iterator             g_const_iterator;
  typedef Groups::mapped_type::const_iterator g_item_iterator;
  typedef g_item_iterator        g_item_const_iterator;

  FieldMap( const message_order& order =
            message_order( message_order::normal ) )
  : m_order( order ), m_fields( order ) {}

  FieldMap( const allocator_type& a, const message_order& order =
            message_order( message_order::normal ) )
  : m_order( order ), m_fields( order, a ) {}

  FieldMap( const int order[] )
  : m_order( order ), m_fields( m_order ) {}

  FieldMap( const FieldMap& copy )
  : m_fields( copy.m_fields.key_comp() )
  { *this = copy; }

  FieldMap( const allocator_type& a, const FieldMap& copy )
  : m_fields( copy.m_fields.key_comp(), a )
  { *this = copy; }

  virtual ~FieldMap();

  FieldMap& operator=( const FieldMap& rhs );

  /// Adds a field without type checking by constructing in place
  template <typename Packed>
  iterator addField( const Packed& packed,
                     typename Packed::result_type* = NULL )
  {
#ifdef HAVE_BOOST
    Fields::iterator i = m_fields.emplace_hint( m_fields.end(),
                                                packed.getField(), packed );
#else
    Fields::iterator i = m_fields.insert( m_fields.end(),
	Fields::value_type( packed.getField(), FieldBase( packed.getField() ) ) );
    i->second.setPacked( packed );
#endif
    return i;
  }

  iterator addField( const FieldBase& field )
  {
#ifdef HAVE_BOOST
    return m_fields.emplace_hint( m_fields.end(), field.getField(), field );
#else
    return m_fields.insert( m_fields.end(),
	Fields::value_type( field.getField(), field ) );
#endif
  }

  template <typename Packed>
  iterator addField( FieldMap::iterator hint, const Packed& packed,
                     typename Packed::result_type* = NULL )
  {
#ifdef HAVE_BOOST
    return m_fields.emplace_hint( hint, packed.getField(), packed );
#else
    Fields::iterator i = m_fields.insert( Fields::value_type( packed.getField(),
                                          FieldBase( packed.getField() ) ) );
    i->second.setPacked( packed );
    return i;
#endif
  }

  iterator addField( FieldMap::iterator hint, const FieldBase& field )
  {
#ifdef HAVE_BOOST
    return m_fields.emplace_hint( hint, field.getField(), field );
#else
    return m_fields.insert( Fields::value_type( field.getField(), field ) );
#endif
  }

  /// Overwrite a field without type checking by constructing in place
  template <typename Packed>
  FieldBase& setField( const Packed& packed,
                       typename Packed::result_type* = NULL)
  {
    int tag = packed.getField();
    Fields::iterator i =  m_fields.lower_bound( tag );
    if ( i == m_fields.end() || i->first != tag )
#ifdef HAVE_BOOST
      i = m_fields.emplace_hint( i, tag, packed );
    else
#else
      i = m_fields.insert( i,
	Fields::value_type( tag, FieldBase( tag ) ) );
#endif
    i->second.setPacked( packed );
    return i->second;
  }

  /// Set a field without type checking
  void setField( const FieldBase& field, bool overwrite = true )
  throw( RepeatedTag )
  {
    Fields::iterator i;
    int tag = field.getField();
    if ( overwrite ) {
      i = m_fields.lower_bound( tag );
      if ( i != m_fields.end() && i->first == tag )
        i->second = field;
      else
#ifdef HAVE_BOOST
        m_fields.emplace_hint( i, tag, field );
#else
        m_fields.insert( i, Fields::value_type( tag, field ) );
#endif
    }
    else
      m_fields.insert( Fields::value_type( tag, field ) );
  }

  /// Set a field without a field class
  void setField( int tag, const std::string& value )
  throw( RepeatedTag, NoTagValue )
  {
    Fields::iterator i = m_fields.lower_bound( tag );
    if ( i == m_fields.end() || i->first != tag )
    {
      i = m_fields.insert( i,
	Fields::value_type( tag, FieldBase( tag ) ) );
    }
    i->second.setString( value );
  }

  /// Get a field if set
  bool getFieldIfSet( FieldBase& field ) const
  {
    Fields::const_iterator iter = m_fields.find( field.getField() );
    if ( iter == m_fields.end() )
      return false;
    field = iter->second;
    return true;
  }

  /// Get a field without type checking
  FieldBase& getField( FieldBase& field )
  const throw( FieldNotFound )
  {
    Fields::const_iterator iter = m_fields.find( field.getField() );
    if ( iter == m_fields.end() )
      throw FieldNotFound( field.getField() );
    field = iter->second;
    return field;
  }

  /// Get a field without a field class
  const std::string& getField( int tag )
  const throw( FieldNotFound )
  {
    return getFieldRef( tag ).getString();
  }

  /// Get direct access to a field through a reference
  const FieldBase& getFieldRef( int tag )
  const throw( FieldNotFound )
  {
    Fields::const_iterator iter = m_fields.find( tag );
    if ( iter == m_fields.end() )
      throw FieldNotFound( tag );
    return iter->second;
  }

  /// Get direct access to a field through a pointer
  const FieldBase* getFieldPtr( int tag )
  const throw( FieldNotFound )
  {
    return &getFieldRef( tag );
  }

  /// Get direct access to a field through a pointer
  const FieldBase* getFieldPtrIfSet( int tag ) const
  {
    Fields::const_iterator iter = m_fields.find( tag );
    return ( iter != m_fields.end() ) ? &iter->second : NULL;
  }

  /// Check to see if a field is set
  bool isSetField( const FieldBase& field ) const
  { return m_fields.find( field.getField() ) != m_fields.end(); }
  /// Check to see if a field is set by referencing its number
  bool isSetField( int tag ) const
  { return m_fields.find( tag ) != m_fields.end(); }

  /// Remove a field. If field is not present, this is a no-op.
  void removeField( int tag );

  /// Remove a range of fields 
  template <typename TagIterator>
  void removeFields(TagIterator tb, TagIterator te)
  {
    if( tb < te )
    {
      int tag, last = *(tb + ((te - tb) - 1));
      Fields::iterator end = m_fields.end();
      Fields::iterator it = m_fields.lower_bound(*tb);
      while( it != end && (tag = it->first) <= last )
      {
        for( ; *tb < tag; ++tb )
          ;
        if( tag == *tb )
        {
          Fields::iterator removed = it++;
          m_fields.erase(removed);
        }
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
  {
    return group = getGroupRef( num, tag );
  }

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
  {
    return &getGroupRef( num, tag );
  }

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
  void clear();
  /// Check if map contains any fields
  bool isEmpty();

  int totalFields() const;

  std::string& calculateString( std::string&, bool clear = true ) const;

  int calculateLength( int beginStringField = FIELD::BeginString,
                       int bodyLengthField = FIELD::BodyLength,
                       int checkSumField = FIELD::CheckSum ) const;

  int calculateTotal( int checkSumField = FIELD::CheckSum ) const;

  iterator begin() const { return m_fields.begin(); }
  iterator end() const { return m_fields.end(); }
  g_iterator g_begin() const { return m_groups.begin(); }
  g_iterator g_end() const { return m_groups.end(); }

  template <typename S> S& serializeTo( S& sink ) const
  {
    Fields::const_iterator i;
    for ( i = m_fields.begin(); i != m_fields.end(); ++i )
    {
      i->second.pushValue(sink);

      if( !m_groups.size() ) continue;
      Groups::const_iterator j = m_groups.find( i->first );
      if ( j == m_groups.end() ) continue;
      GroupItem::const_iterator k;
      for ( k = j->second.begin(); k != j->second.end(); ++k )
        ( *k ) ->serializeTo( sink );
    }
    return sink;
  }

  allocator_type get_allocator()
  {
    return m_fields.get_allocator();
  }

  static inline
  allocator_type create_allocator(std::size_t n = ItemStore::MaxCapacity)
  {
    return allocator_type( ItemStore::buffer(n * AllocationUnit) );
  } 

private:
  message_order m_order; // must be first
  Fields m_fields;
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

