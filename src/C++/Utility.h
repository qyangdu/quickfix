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

#ifndef FIX_UTILITY_H
#define FIX_UTILITY_H

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#endif

#ifndef _MSC_VER
#include "Config.h"
#endif

#ifdef HAVE_STLPORT
  #define ALLOCATOR std::allocator
#elif ENABLE_DEBUG_ALLOCATOR
  #include <ext/debug_allocator.h>
  #define ALLOCATOR __gnu_cxx::debug_allocator
#elif ENABLE_NEW_ALLOCATOR
  #include <ext/new_allocator.h>
  #define ALLOCATOR __gnu_cxx::new_allocator
#elif ENABLE_BOOST_FAST_POOL_ALLOCATOR
  #include <boost/pool/pool_alloc.hpp>
  #define ALLOCATOR boost::fast_pool_allocator
#elif ENABLE_MT_ALLOCATOR
  #include <ext/mt_allocator.h>
  #define ALLOCATOR __gnu_cxx::__mt_alloc
#elif ENABLE_BOOST_POOL_ALLOCATOR
  #include <boost/pool/pool_alloc.hpp>
  #define ALLOCATOR boost::pool_allocator
#elif ENABLE_POOL_ALLOCATOR
  #include <ext/pool_allocator.h>
  #define ALLOCATOR __gnu_cxx::__pool_alloc
#elif ENABLE_BITMAP_ALLOCATOR
  #include <ext/bitmap_allocator.h>
  #define ALLOCATOR __gnu_cxx::bitmap_allocator
#elif ENABLE_TBB_ALLOCATOR
  #undef VERSION
  #include <tbb/scalable_allocator.h>
  #include <tbb/cache_aligned_allocator.h>
  #define ALLOCATOR tbb::scalable_allocator
#else
  #define ALLOCATOR std::allocator
#endif

#include <limits>
#include <string>
#include <stdexcept>

#ifdef _MSC_VER
/////////////////////////////////////////////
#include <stddef.h>
#include <Winsock2.h>
#include <process.h>
#include <malloc.h>
#include <direct.h>
#include <time.h>
#include <xmmintrin.h>
#if _MSC_VER < 1600
  typedef signed __int8     int8_t;
  typedef signed __int16    int16_t;
  typedef signed __int32    int32_t;
  typedef unsigned __int8   uint8_t;
  typedef unsigned __int16  uint16_t;
  typedef unsigned __int32  uint32_t;
  typedef signed __int64    int64_t;
  typedef unsigned __int64  uint64_t;
#else
#include <stdint.h>
#endif
typedef int socklen_t;
/////////////////////////////////////////////
#else
/////////////////////////////////////////////
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
/////////////////////////////////////////////
#endif

#include <climits>
#include <cstring>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstdlib>

#ifdef HAVE_BOOST
#include <boost/spirit/include/qi_real.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/range.hpp>
#endif

#ifdef HAVE_TBB
  #include <tbb/spin_mutex.h>
  #include <tbb/atomic.h>
#endif

#if defined(_MSC_VER)
#define HAVE_FSCANF_S 1
#define FILE_FSCANF fscanf_s
#else
#define FILE_FSCANF fscanf
#endif

#if defined(_MSC_VER)
#define HAVE_SNPRINTF_S 1
#define STRING_SNPRINTF sprintf_s
#else
#define STRING_SNPRINTF snprintf
#endif

#if defined(_MSC_VER)
#define ALIGN_DECL(x) __declspec(align(x))
#define NOTHROW __declspec(nothrow)
#define NOTHROW_PRE __declspec(nothrow)
#define NOTHROW_POST
#define HEAVYUSE
#define PREFETCH(addr, rw, longevity) _mm_prefetch(addr, longevity)
#define LIKELY(x) (x)
#define MAY_ALIAS
#define PURE
#elif defined(__GNUC__)
#define ALIGN_DECL(x) __attribute__ ((aligned(x)))
#define NOTHROW __attribute__ ((nothrow))
#define NOTHROW_PRE
#define NOTHROW_POST __attribute__ ((nothrow))
#define HEAVYUSE __attribute__((hot))
#define PREFETCH(addr, rw, longevity) __builtin_prefetch(addr, rw, longevity)
#define LIKELY(x) __builtin_expect((x),1)
#define MAY_ALIAS __attribute__ ((may_alias))
#define PURE __attribute__ ((pure))
#else
#define ALIGN_DECL(x)
#define NOTHROW
#define NOTHROW_PRE
#define NOTHROW_POST
#define HEAVYUSE
#define PREFETCH(addr, rw, longevity) while(false)
#define LIKELY(x) (x)
#define MAY_ALIAS
#define PURE
#endif

#define ALIGN_DECL_DEFAULT ALIGN_DECL(16)

namespace FIX
{
  void string_replace( const std::string& oldValue,
                       const std::string& newValue,
                       std::string& value );

  std::string string_toLower( const std::string& value );
  std::string string_toUpper( const std::string& value );
  std::string string_strip( const std::string& value );

  void socket_init();
  void socket_term();
  int socket_createAcceptor( int port, bool reuse = false );
  int socket_createConnector();
  int socket_connect( int s, const char* address, int port );
  int socket_accept( int s );
  int socket_send( int s, const char* msg, int length );
  void socket_close( int s );
  bool socket_fionread( int s, int& bytes );
  bool socket_disconnected( int s );
  int socket_setsockopt( int s, int opt );
  int socket_setsockopt( int s, int opt, int optval );
  int socket_getsockopt( int s, int opt, int& optval );
#ifndef _MSC_VER
  int socket_fcntl( int s, int opt, int arg );
  int socket_getfcntlflag( int s, int arg );
  int socket_setfcntlflag( int s, int arg );
#endif
  void socket_setnonblock( int s );
  bool socket_isValid( int socket );
#ifndef _MSC_VER
  bool socket_isBad( int s );
#endif
  void socket_invalidate( int& socket );
  short socket_hostport( int socket );
  const char* socket_hostname( int socket );
  const char* socket_hostname( const char* name );
  const char* socket_peername( int socket );
  std::pair<int, int> socket_createpair();

  tm time_gmtime( const time_t* t );
  tm time_localtime( const time_t* t );

#ifdef _MSC_VER

#define ALIGNED_ALLOC(p, sz, alignment) (p = (CHAR*)_aligned_malloc(sz, alignment))
#define ALIGNED_FREE(p) _aligned_free(p)

  typedef unsigned int (_stdcall THREAD_START_ROUTINE)(void *);
#define  THREAD_PROC unsigned int _stdcall
  typedef unsigned thread_id;

  typedef HANDLE   FILE_HANDLE_TYPE;
#define INVALID_FILE_HANDLE_VALUE (INVALID_HANDLE_VALUE)
#define FILE_POSITION_SET (FILE_BEGIN)
#define FILE_POSITION_END (FILE_CURRENT)
#define FILE_POSITION_CUR (FILE_END)
  typedef LARGE_INTEGER FILE_OFFSET_TYPE;
#define FILE_OFFSET_TYPE_MOD	      "I64"
#define FILE_OFFSET_TYPE_SET(offt, value) ((offt).QuadPart = (value))
#define FILE_OFFSET_TYPE_VALUE(offt) ((offt).QuadPart)
#define FILE_OFFSET_TYPE_ADDR(offt) (&(offt).QuadPart)

#else /* !_MSC_VER */

#define ALIGNED_ALLOC(p, sz, alignment) posix_memalign(&(p), alignment, sz)
#define ALIGNED_FREE(p) free(p)

  extern "C" { typedef void * (THREAD_START_ROUTINE)(void *); }
#define THREAD_PROC void *
  typedef pthread_t thread_id;

  typedef int	    FILE_HANDLE_TYPE;
#define INVALID_FILE_HANDLE_VALUE (-1)
#define FILE_POSITION_SET (SEEK_SET)
#define FILE_POSITION_END (SEEK_CUR)
#define FILE_POSITION_CUR (SEEK_END)
  typedef off_t   FILE_OFFSET_TYPE;
#define FILE_OFFSET_TYPE_MOD		"L"
#define FILE_OFFSET_TYPE_SET(offt, value) ((offt) = (value))
#define FILE_OFFSET_TYPE_VALUE(offt) ((long long)offt)
#define FILE_OFFSET_TYPE_ADDR(offt) ((long long*)&(offt))
#endif /* _MSC_VER */

  FILE_HANDLE_TYPE file_handle_open( const char* path );
  void file_handle_close( FILE_HANDLE_TYPE );
  long file_handle_write( FILE_HANDLE_TYPE, const char* , std::size_t size );
  long file_handle_read_at( FILE_HANDLE_TYPE, char*, std::size_t size,
                         FILE_OFFSET_TYPE offset );
  FILE_OFFSET_TYPE file_handle_seek( FILE_HANDLE_TYPE,
                                     FILE_OFFSET_TYPE offset, int whence );

  bool thread_spawn( THREAD_START_ROUTINE func, void* var, thread_id& thread );
  bool thread_spawn( THREAD_START_ROUTINE func, void* var );
  void thread_join( thread_id thread );
  void thread_detach( thread_id thread );
  thread_id thread_self();

  void process_sleep( double s );

  std::string file_separator();
  void file_mkdir( const char* path );
  std::FILE* file_fopen( const char* path, const char* mode );
  void file_fclose( std::FILE* file );
  bool file_exists( const char* path );
  void file_unlink( const char* path );
  int file_rename( const char* oldpath, const char* newpath );
  std::string file_appendpath( const std::string& path, const std::string& file );

  namespace detail
  {
    template <size_t N> struct log_
    {
      enum { value = 1 + log_<N/2>::value };
    };

    template <> struct log_<1>
    {
      enum { value = 0 };
    };

    template <> struct log_<0>
    {
      enum { value = 0 };
    };

    struct bitop_base
    {
      static const int Mod67Position[];
    };

    template <typename W, std::size_t S = sizeof(W)>
    struct bitop : public bitop_base
    {
      static inline std::size_t bsf(W w)
      {
        register const W notfound = (W)(S << 3);
        std::size_t v = Mod67Position[((1 + ~(uint64_t)w) & (uint64_t)w) % 67];
        return (v != 64) ? v : notfound;
      }
    };

#if defined (__GNUC__)
#  if defined (__x86_64__)
    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        register std::size_t v;
        register const W notfound = 64;
        __asm__ __volatile__ (
          "bsf %1, %0; "
          "cmovz %2, %0"
          : "=r" (v)
          : "rm" (w), "r"(notfound)
          : "cc");
        return v;
      }
    };
    
    template <typename W> struct bitop<W, 4>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        register unsigned v;
        register const W notfound = 32;
        __asm__ __volatile__ (
          "bsf %1, %0; "
          "cmovz %2, %0"
          : "=r" (v)
          : "rm" (w), "r"(notfound)
          : "cc");
        return v;
      }
    };
#  else
    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        register std::size_t v = __builtin_ffsll((uint64_t)w);
        return v ? (v - 1) : 64;
      }
    };
    
    template <typename W> struct bitop<W, 4>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        register std::size_t v = __builtin_ffs((uint32_t)w);
        return v ? (v - 1) : 32;
      }
    };
#  endif
#elif defined(_MSC_VER)

    template <typename W> struct bitop<W, 4>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        unsigned long v;
        return _BitScanForward(&v, (unsigned long)w) ? v : 32;
      }
    };

#  if defined(_M_X64)

    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        unsigned long v;
        return _BitScanForward64(&v, (unsigned __int64)w) ? v : 64;
      }
    };

#  elif defined(_M_IX86)

    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE HEAVYUSE bsf(W w)
      {
        unsigned long v;
        return _BitScanForward( &v, (unsigned long) w ) ? v
             : _BitScanForward( &v, (unsigned long)(w >> 32) ) ? (v + 32) : 64;
      }
    };

#  endif
#endif

    template <typename W>
    static inline std::size_t PURE HEAVYUSE bit_scan(W w, std::size_t from = 0)
    {
      return bitop<W>::bsf( w & (~(W)0 << from) );
    }

  } // namespace detail

  namespace Util {

#ifdef HAVE_BOOST
    template <typename T> struct scoped_array {
        typedef boost::scoped_array<T> type;
    };
#else
    template <typename T> struct scoped_array {
      class type {
        T* p_;
      public:
        type(T* p = NULL) : p_(p) {}
        T* get() const { return p_; }
        ~type() { if (p_) delete [] p_; }
      };
    };
#endif
  
#if defined(__GNUC__) && defined(__x86_64__)
    class IntBase {
      protected:
      struct Log2 {
        int32_t m_threshold;
        int32_t m_count;
      };
      static Log2 m_digits[32];
    };

    class Int : public IntBase {
      public:
      static inline int PURE HEAVYUSE numDigits( int32_t i )
      {
	register int32_t z = 0;
	register uint32_t log2;
	if (i < 0) i = -i;
	__asm__ __volatile__ (
		"bsr %1, %0;"  \
		"cmovz %2, %0;"\
		: "=r" (log2)  \
		: "rm" (i), "r"(z));
	register const Log2& e = m_digits[log2];
	return e.m_count + ( i > e.m_threshold );
      }
    };
    class PositiveInt : public IntBase {
      public:
      static inline int PURE HEAVYUSE numDigits( int32_t i )
      {
        register uint32_t log2;
        __asm__ __volatile__ (
                "bsr %1, %0; " \
                : "=r" (log2)  \
                : "rm" (i));
        register const Log2& e = m_digits[log2];
        return e.m_count + ( i > e.m_threshold );
      }
      static inline short PURE HEAVYUSE checkSum(int n)
      {
	short csum = 0;
        do
        {
          csum += (int)'0' + n % 10;
        } while ( (n /= 10) );
        return csum;
      }
    };
    class Long {
      struct Log2 {
        int64_t m_threshold;
        int64_t m_count;
      };
      static Log2 m_digits[64];
      public:
      static inline int PURE HEAVYUSE numDigits( int64_t i )
      {
        register const int64_t z = 0;
        register uint64_t log2;
        if (i < 0) i = -i;
        __asm__ __volatile__ (
                "bsr %1, %0;"  \
                "cmovz %2, %0;"\
                : "=r" (log2)  \
                : "rm" (i), "r"(z));
  
        register const Log2& e = m_digits[log2];
        return e.m_count + ( i > e.m_threshold );
      }
    };
#else
    class Int {
      public:
      static inline int PURE HEAVYUSE numDigits( int32_t i )
      {
	if (i < 0)  i = -i;
	if              (i < 100000) {
	    if          (i < 1000) {
	        if      (i < 10)         return 1;
	        else if (i < 100)        return 2;
	        else                     return 3;
	    } else {
	        if      (i < 10000)      return 4;
	        else                     return 5;
	    }
	} else {
	    if          (i < 10000000) {
	        if      (i < 1000000)    return 6;
	        else                     return 7;
	    } else {
	        if      (i < 100000000)  return 8;
	        else if (i < 1000000000) return 9;
	        else                     return 10;
	    }
	}
      }
    };
    class PositiveInt {
      public:
      static inline int PURE HEAVYUSE numDigits( int32_t i )
      {
	if              (i < 100000) {
	    if          (i < 1000) {
	        if      (i < 10)         return 1;
	        else if (i < 100)        return 2;
	        else                     return 3;
	    } else {
	        if      (i < 10000)      return 4;
	        else                     return 5;
	    }
	} else {
	    if          (i < 10000000) {
	        if      (i < 1000000)    return 6;
	        else                     return 7;
	    } else {
	        if      (i < 100000000)  return 8;
	        else if (i < 1000000000) return 9;
	        else                     return 10;
	    }
	}
      }
      static inline short PURE HEAVYUSE checkSum(int n)
      {
	short csum = 0;
        do
        {
          csum += (int)'0' + n % 10;
        } while ( (n /= 10) );
        return csum;
      }
    };
    class Long {
      public:
      static inline int PURE HEAVYUSE numDigits( int64_t v )
      {
        int r = 1;
        if (v < 0) v = -v;
	if (v >= 10000000000000000LL)
        {
	  r += 16;
	  v /= 10000000000000000LL;
        }
        if (v >= 100000000)
        {
          r += 8;
          v /= 100000000;
        }

        if (v >= 10000) {
          r += 4;
          v /= 10000;
        }

        if (v >= 100) {
          r += 2;
          v /= 100;
        }
        return r + (v >= 10);
      }
    };
#endif
    class ULong {
      public:
      static inline int PURE HEAVYUSE numFractionDigits( uint64_t v )
      {
        if            ( v % 100000000 ) {
          if          ( v % 10000 ) {
            if        ( v % 100 ) {
              if      ( v % 10 )                 return 16;
              else                               return 15;
            } else if ( v % 1000 )               return 14;
              else                               return 13;
          } else {
            if        ( v % 1000000 ) {
              if      ( v % 100000 )             return 12;
              else                               return 11;
            } else if ( v % 10000000 )           return 10;
                 else                            return 9;
          }
        } else {
          if          ( v % 1000000000000LL ) {
            if        ( v % 10000000000LL ) {
              if      ( v % 1000000000 )         return 8;
              else                               return 7;
            } else if ( v % 100000000000LL )     return 6;
                   else                          return 5;
          } else {
            if        ( v % 100000000000000LL ) {
              if      ( v % 10000000000000LL )   return 4;
              else                               return 3;
            } else if ( v % 1000000000000000LL ) return 2;
                   else                          return 1;
          }
        }
        return 0;
      }
    };

    struct CharBuffer
    {
      class reverse_iterator : public std::iterator<std::input_iterator_tag, char>
      {
          char* m_p;
        public:
          reverse_iterator(char* p) : m_p(p - 1) {}
          reverse_iterator operator++() { return --m_p; }
          reverse_iterator operator++(int) { return m_p--; }
          char operator*() const { return *m_p; }
          bool operator==( reverse_iterator it) const
          { return m_p == it.m_p; }
          bool operator!=( reverse_iterator it) const
          { return m_p != it.m_p; }
          difference_type operator-( reverse_iterator it) const
          { return it.m_p - m_p; }
      };

      static inline const char* memmem(const char* buf, int bsz,
                                       const char* str, int ssz)
      {
#if defined(__GNUC__)
	return (const char*)::memmem(buf, bsz, str, ssz);
#else
        const char* end = buf + bsz;
        for ( buf = ::memchr (buf, str[0], bsz) ; buf && bsz >= ssz;
              buf = ::memchr (buf, str[0], bsz) )
        {
            if( 0 == ::memcmp (buf, str, ssz) ) 
              return buf; 
            bsz = end - ++buf;
          }
        }
        return 0; 
#endif
      }

      static inline int32_t PURE HEAVYUSE checkSum(const char* p, int size)
      {
        register int32_t x = 0;
#if defined(__GNUC__) && defined(__x86_64__)
        register int32_t n = size;
        register const uint8_t* pu = (const uint8_t*)p;
        register const uint64_t mask = 0x0101010101010101ULL;

        if (n >= 8)
              __asm__ __volatile (
              " cmpl $16, %2;             \n\t"
              " pxor %%xmm0, %%xmm0;      \n\t"
              " movdqa %%xmm0, %%xmm2;    \n\t"
              " jl 1f;                \n\t"
              " movq %3, %%xmm1;          \n\t"
              " punpcklbw %%xmm0, %%xmm1; \n\t"
              " .p2align 4,,10;           \n\t"
              " .p2align 3;               \n\t"
              // loop over 16-byte blocks
              "0:;                    \n\t"
              " movdqu (%1), %%xmm4;      \n\t"
              " movdqa %%xmm4, %%xmm3;    \n\t"
              " addl $-16, %2;            \n\t"
              " punpcklbw %%xmm2, %%xmm3; \n\t"
              " addq $16, %1;             \n\t"
              " pmaddwd %%xmm1, %%xmm3;   \n\t"
              " cmpl $16, %2;             \n\t"
              " punpckhbw %%xmm2, %%xmm4; \n\t"
              " paddd %%xmm3, %%xmm0;     \n\t"
              " pmaddwd %%xmm1, %%xmm4;   \n\t"
              " paddd %%xmm4, %%xmm0;     \n\t"
              " jge 0b;               \n\t"
              " cmpl $4, %2;              \n\t"
              " jl 2f;                \n\t"
              " .p2align 4,,10;           \n\t"
              " .p2align 3;               \n\t"
              // loop over 4-byte blocks
              "1:                     \n\t"
              " movd (%1), %%xmm3;        \n\t"
              " addq $4, %1;              \n\t"
              " punpcklbw %%xmm2, %%xmm3; \n\t"
              " addl $-4, %2;             \n\t"
              " punpcklwd %%xmm2, %%xmm3; \n\t"
              " cmpl $4, %2;              \n\t"
              " paddd %%xmm3, %%xmm0;     \n\t"
              " jge 1b;               \n\t"
              // accumulate
              "2:;                    \n\t"
              " movdqa %%xmm0, %%xmm1;    \n\t"
              " psrldq $8, %%xmm1;        \n\t"
              " paddd %%xmm1, %%xmm0;     \n\t"
              " movdqa %%xmm0, %%xmm2;    \n\t"
              " psrldq $4, %%xmm2;        \n\t"
              " paddd %%xmm2, %%xmm0;     \n\t"
              " movd %%xmm0, %0;          \n\t"
              : "+r"(x), "+r"(pu), "+r"(n)
              : "r" (mask)
              : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4"
              );

        while(n > 0)
        {
            // loop over remaining bytes
            __asm__ __volatile__ (
            " movzbl (%1), %%ecx; "
            " addl %%ecx, %0;   "
            : "+r" (x), "+r" (pu)
            :
            : "ecx" );
            pu++;
            n--;
        }
#else
        for (; size > 0; p++, n--)
          x += *(unsigned char*)p;
#endif
        return x;
      }

      template <std::size_t S> union Fixed
      {
        char data[S];
      };

      /// Returns array index less than S on success and a value of S or higher on failure
      template <std::size_t S> static inline std::size_t find(const Fixed<S>& f, int v)
      {
        const char* p = (const char*)::memchr(f.data, v, S);
        return p ? p - f.data : S;
      }

    }; // CharBuffer

#if defined(__GNUC__) && defined(__x86_64__)

    template <> union CharBuffer::Fixed<4>
    {
      char data[4];
      uint32_t value;
    };

    template <> union CharBuffer::Fixed<8>
    {
      char data[8];
      uint64_t value;
    };

    template<> inline std::size_t PURE CharBuffer::find<8>(const CharBuffer::Fixed<8>& f, int v)
    {
      register unsigned at = 0;
      __asm__ __volatile (
        "movd %1, %%xmm1            \n\t"
        "punpcklbw %%xmm1, %%xmm1   \n\t"
        "movq %2, %%xmm0            \n\t"
        "pshuflw $0, %%xmm1, %%xmm1 \n\t"
        "movl $8, %1                \n\t"
        "pcmpeqb %%xmm1, %%xmm0     \n\t"
        "pmovmskb %%xmm0, %0        \n\t"
        "bsf %0, %0                 \n\t"
        "cmovz %1, %0               \n\t"
        : "=a" (at), "+r" (v)
        : "r" (f.value)
        : "xmm0", "xmm1" );
      return at;
    }

#endif
  
    struct Tag {

      /* NOTE: Current tag value range is 1-39999, may not contain leading zeroes */

      static inline const char NOTHROW_PRE * PURE HEAVYUSE NOTHROW_POST delimit(const char* p, unsigned len)
      {
        uintptr_t b = ~(uintptr_t)p + 1;
        switch (len)
        {
          default: b = (p[7] == '=') ? 7 : b;
          case 7:  b = (p[6] == '=') ? 6 : b;
          case 6:  b = (p[5] == '=') ? 5 : b;
          case 5:  b = (p[4] == '=') ? 4 : b;
          case 4:  b = (p[3] == '=') ? 3 : b;
          case 3:  b = (p[2] == '=') ? 2 : b;
          case 2:  b = (p[1] == '=') ? 1 : b;
          case 1:  {}
          case 0:  {}
        }
        return p + b;
      }

      static inline bool HEAVYUSE NOTHROW parse(const char* b, const char* e, int& rv, int& rx)
      {
        register bool result = true;
        int value = 0, checksum = 0;
        do
        {
          int v = *b;
          result = (v >= '0' && v <= '9');
          value = value * 10 + (v - '0');
          checksum += v;
        } while (LIKELY((++b < e) & result));
        rv = value; rx = checksum;
        return result;
      }

      static inline char NOTHROW length(int field)
      {
        return Util::PositiveInt::numDigits(field) + 1;
      }

      static inline short NOTHROW checkSum(int field)
      {
        return Util::PositiveInt::checkSum(field) + (short)'=';
      }

    }; // Tag

    template <std::size_t N> class BitSet {

      typedef unsigned long word_type;

      static const std::size_t word_size = sizeof(word_type) * 8;
      static const std::size_t word_shift = detail::log_<sizeof(word_type) * 8>::value;
      static const std::size_t Width = N/word_size + ((N%word_size) ? 1 : 0);

      word_type m_bits[Width];

      std::size_t _Find_from( int word )
      {
        for(; word < Width; word++)
        {
          std::size_t b = detail::bit_scan(m_bits[word]);
          if (word_size != b)
            return ((word << word_shift) + b);
        }
        return this->size();
      }

    public:
      BitSet() { this->reset(); }
      BitSet( bool v ) { if ( v ) this->set(); else this->reset(); }

      BitSet NOTHROW_PRE & NOTHROW_POST reset()
      {
        ::memset( m_bits, 0, sizeof(m_bits) );
        return *this;
      }
      BitSet NOTHROW_PRE & NOTHROW_POST reset( int bit )
      {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
 	__asm__ __volatile__ (
        "btr %1, %0; "
        : "+m" (*(volatile word_type*)m_bits)
        : "Ir" (bit)
        : "cc", "memory");   
#elif defined(_MSC_VER)
        _bittestandreset( (long*)m_bits, bit );
#else
	m_bits[bit >> word_shift] &= ~((word_type)1 << (bit & (word_size - 1)));
#endif
        return *this;
      }

      inline BitSet NOTHROW_PRE & NOTHROW_POST set()
      {
        ::memset( m_bits, 255, sizeof(m_bits) );
        if ( N%word_size )
        {
          word_type bits = (~(word_type)0) >> (word_size - N%word_size);
          m_bits[Width - 1] = bits;
        }
        return *this;
      }
      inline BitSet NOTHROW_PRE & NOTHROW_POST set( int bit )
      {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
 	__asm__ __volatile__ (
        "bts %1, %0; "
        : "+m" (*(volatile word_type*)m_bits)
        : "Ir" (bit)
        : "cc", "memory");   
#elif defined(_MSC_VER)
        _bittestandset( (long*)m_bits, bit );
#else
        m_bits[bit >> word_shift] |= ((word_type)1 << (bit & (word_size - 1)));
#endif
        return *this;
      }
      inline BitSet NOTHROW_PRE & NOTHROW_POST set( int bit, bool value )
      {
        return value ? set( bit ) : reset( bit );
      }

      inline bool NOTHROW operator[]( int bit ) const
      {
        unsigned char nRet;
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
        __asm__ __volatile__ (
        "bt %1, %2; "
        "setc %b0 "                                     
        : "=r" (nRet)                                   
        : "Ir" (bit), "m"(*(volatile word_type*)m_bits) 
        : "cc");
        return nRet;
#elif defined(_MSC_VER)
        nRet = _bittest( (long*)m_bits, bit );
        return nRet != 0;
#else
        nRet = (m_bits[bit >> word_shift] &
               ((word_type)1 << (bit & (word_size - 1)))) ? 1 : 0;
        return nRet != 0;
#endif
      }

      inline int NOTHROW _Find_first()
      {
        return _Find_from(0);
      }

      int _Find_next( int b )
      {
        std::size_t i = b >> word_shift;
        std::size_t v = detail::bit_scan(m_bits[i], b & (word_size - 1));
        return (word_size != v) ? ((i << word_shift) + v) : _Find_from(++i);
      }

      inline BitSet NOTHROW_PRE & NOTHROW_POST operator <<=(int n)
      {
        if (n <= N)
        {
          const int offset = n / word_size;
          const int shift  = n - (offset * word_size);
          int dst = Width - 1, src = Width - 1 - offset;
          word_type rem = m_bits[src--] << shift;
          for ( ; src >= 0; dst--, src-- )
          {
            register word_type w = m_bits[src];
            m_bits[dst] = rem | (w >> (word_size - shift));
            rem = w << shift;
          }
          m_bits[dst] = rem;
          ::memset( m_bits, 0, offset * sizeof(word_type) );
          if ( N%word_size )
          {
            rem = (~(word_type)0) >> (word_size - N%word_size);
            m_bits[Width - 1] &= rem;
          }
          return *this;
        }
        return reset();
      }

      inline BitSet NOTHROW_PRE & NOTHROW_POST operator >>=(int n)
      {
        if (n <= N)
        {
          const int offset = n / word_size;
          const int shift  = n - (offset * word_size);
          int dst = 0, src = offset;
          word_type rem = m_bits[src++] >> shift;
          for ( ; src < Width; dst++, src++ )
          {
            register word_type w = m_bits[src];
            m_bits[dst] = rem | (w << (word_size - shift));
            rem = w >> shift;
          }
          m_bits[dst++] = rem;
          ::memset(m_bits + dst, 0, offset * sizeof(word_type) );
          return *this;
        }
        return reset();
      }

      std::size_t size() const { return N; }

    }; // template class BitSet

    template <> class BitSet<sizeof(uint64_t) * 8>
    {
      typedef uint64_t word_type;

      word_type m_bits;

    public:
      BitSet() { reset(); }
      BitSet( bool v ) { m_bits = v ? ~(word_type)0 : 0; }

      BitSet NOTHROW_PRE & NOTHROW_POST reset() { m_bits = 0; return *this; }
      BitSet NOTHROW_PRE & NOTHROW_POST reset( int bit )
      { 
	m_bits &= ~((word_type)1 << bit);
        return *this;
      }

      inline BitSet NOTHROW_PRE & NOTHROW_POST set()
      {
        m_bits = ~(word_type)0;
        return *this;
      }
      inline BitSet NOTHROW_PRE & NOTHROW_POST set( int bit )
      {
        m_bits |= ((word_type)1 << bit);
        return *this;
      }
      inline BitSet NOTHROW_PRE & NOTHROW_POST set( int bit, bool value )
      {
        return value ? set( bit ) : reset( bit );
      }

      inline bool NOTHROW operator[]( int bit ) const
      {
        return ((m_bits & ((word_type)1 << bit)) != 0);
      }

      inline int NOTHROW _Find_first()
      {
        return detail::bit_scan(m_bits);
      }

      int _Find_next( int b )
      {
        return detail::bit_scan(m_bits, b);
      }

      inline BitSet NOTHROW_PRE & NOTHROW_POST operator <<=(int n) { m_bits <<= n; return *this; }

      inline BitSet NOTHROW_PRE & NOTHROW_POST operator >>=(int n) { m_bits >>= n; return *this; }

      std::size_t size() const { return sizeof(word_type) << 3; }
    };

    // Debugging aid
    bool checkpoint();

  } // namespace FIX::Util

  struct Sg
  {

#ifdef _MSC_VER
    typedef WSABUF   sg_buf_t;
    typedef LPWSABUF sg_buf_ptr;
    typedef CHAR*    sg_ptr_t;

#define IOV_BUF(b) ((b).buf)
#define IOV_LEN(b) ((b).len)

    static inline std::size_t NOTHROW send(SOCKET socket, sg_buf_ptr bufs, int n)
    {
      DWORD bytes;
      int i = ::WSASend(socket, bufs, n, &bytes, 0, NULL, NULL);
      return (i == 0) ? bytes : 0;
    }

    static inline int64_t NOTHROW writev(FILE_HANDLE_TYPE handle,
                                         sg_buf_ptr bufs, int n)
    {
      int64_t sz = 0;
      for( int i = 0; i < n; i++ )
      {
        DWORD written = 0;
        if( !::WriteFile( handle, IOV_BUF(bufs[i]),
                          IOV_LEN(bufs[i]), &written, NULL ) )
        {
          if( sz == 0 )
            return -1;
          break;
        }
        sz += (int64_t)written;
      }
      return sz;
    }

#else
    typedef struct iovec  sg_buf_t;
    typedef struct iovec* sg_buf_ptr;
    typedef void*         sg_ptr_t;

#define IOV_BUF(b) ((b).iov_base)
#define IOV_LEN(b) ((b).iov_len)

    static inline std::size_t NOTHROW send(int socket, sg_buf_ptr bufs, int n)
    {
      struct msghdr m = { NULL, 0, bufs, (std::size_t)n, NULL, 0, 0 };
      ssize_t sz = ::sendmsg(socket, &m, MSG_NOSIGNAL);
      return sz >= 0 ? (std::size_t)sz : 0;
    }

    static inline int64_t NOTHROW writev(FILE_HANDLE_TYPE handle,
                                         sg_buf_ptr bufs, int n)
    {
      return ::writev(handle, bufs, n);
    }

#endif

    template <typename B>
    static std::size_t NOTHROW size( const B bufs, int n )
    {
      size_t sz = 0;
      for (int i = 0; i < n; i++)
       	sz += IOV_LEN(bufs[i]);
      return sz;
    }

    template <typename B>
    static std::string toString( const B bufs, int n )
    {
      std::string s;
      for (int i = 0; i < n; i++)
       	s.append((const char*)IOV_BUF(bufs[i]), IOV_LEN(bufs[i]));
      return s;
    }
  
  }; // struct Sg

} // namespace FIX

#if (!defined(_MSC_VER) || (_MSC_VER >= 1300)) && !defined(HAVE_STLPORT)
using std::abort;
using std::sprintf;
using std::atoi;
using std::atol;
using std::atof;
using std::isdigit;
using std::strcmp;
using std::strftime;
using std::strlen;
using std::abs;
using std::labs;
using std::memcpy;
using std::memset;
using std::exit;
using std::strtod;
using std::strtol;
using std::strerror;
#endif

#endif
