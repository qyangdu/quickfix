/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) 2001-2014
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
typedef int ssize_t;
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
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <xmmintrin.h>
#ifdef __AVX__
#include <immintrin.h>
#endif
#endif
#if defined(__MACH__)
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#include <mach/mach_time.h>
#include <dispatch/dispatch.h>
#endif
/////////////////////////////////////////////
#endif

#include <climits>
#include <cstring>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <memory>

#ifdef HAVE_BOOST
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/range.hpp>
#endif

#ifdef HAVE_BOOST
  namespace ptr = boost;
#elif defined(HAVE_STD_SHARED_PTR)
  namespace ptr = std;
#elif defined(HAVE_STD_TR1_SHARED_PTR)
  #include <tr1/memory>
  namespace ptr = std::tr1;
#else
  namespace ptr = std;
#endif

#ifdef HAVE_TBB
#include <tbb/tick_count.h>
#include <tbb/spin_mutex.h>
#include <tbb/atomic.h>
#endif

 #ifdef _MSC_VER
#pragma warning( disable : 4706 ) // DISABLE warning C4706: assignment within conditional expression
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
  #define LIGHTUSE
  #define HEAVYUSE
  #define PREFETCH(addr, rw, longevity) _mm_prefetch(addr, longevity)
  #define LIKELY(x) (x)
  #define UNLIKELY(x) (x)
  #define MAY_ALIAS
  #define PURE_DECL
  #ifndef FORCE_INLINE
    #define FORCE_INLINE
  #endif
#elif defined(__GNUC__)
  #define ALIGN_DECL(x) __attribute__ ((aligned(x)))
  #define NOTHROW __attribute__ ((nothrow))
  #define NOTHROW_PRE
  #define NOTHROW_POST __attribute__ ((nothrow))
  #define HEAVYUSE __attribute__((hot))
  #define LIGHTUSE __attribute__((cold))
  #define PREFETCH(addr, rw, longevity) __builtin_prefetch(addr, rw, longevity)
  #define LIKELY(x) __builtin_expect((x),1)
  #define UNLIKELY(x) __builtin_expect((x),0)
  #define MAY_ALIAS __attribute__ ((may_alias))
  #define PURE_DECL __attribute__ ((pure))
  #if defined(__INTEL_COMPILER)
    #define FORCE_INLINE __forceinline
  #else
    #ifndef FORCE_INLINE
      #define FORCE_INLINE
    #endif
  #endif
#else
  #define ALIGN_DECL(x)
  #define NOTHROW
  #define NOTHROW_PRE
  #define NOTHROW_POST
  #define HEAVYUSE
  #define LIGHTUSE 
  #define PREFETCH(addr, rw, longevity) while(false)
  #define LIKELY(x) (x)
  #define UNLIKELY(x) (x)
  #define MAY_ALIAS
  #define PURE_DECL
  #define FORCE_INLINE
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
  ssize_t socket_send( int s, const char* msg, size_t length );
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

  template<typename InputIterator, typename OutputIterator>
  OutputIterator copy_to_array(InputIterator first, InputIterator last, OutputIterator result, size_t result_size)
  {
#ifdef _MSC_VER
    stdext::checked_array_iterator<OutputIterator> checked_array(result, result_size);
    return std::copy(first, last, checked_array).base();
#else
    return std::copy(first, last, result);
#endif
  }

#ifdef _MSC_VER

#define ALIGNED_ALLOC(p, sz, alignment) (p = (CHAR*)_aligned_malloc(sz, alignment))
#define ALIGNED_FREE(p) _aligned_free(p)

  typedef unsigned int (_stdcall THREAD_START_ROUTINE)(void *);
#define  THREAD_PROC unsigned int _stdcall
  typedef unsigned thread_id;

  typedef HANDLE   FILE_HANDLE_TYPE;
#define INVALID_FILE_HANDLE_VALUE (INVALID_HANDLE_VALUE)
#define FILE_POSITION_SET (FILE_BEGIN)
#define FILE_POSITION_CUR (FILE_CURRENT)
#define FILE_POSITION_END (FILE_END)
  typedef LARGE_INTEGER FILE_OFFSET_TYPE;
#define FILE_OFFSET_TYPE_MOD	      "I64"
#define FILE_OFFSET_TYPE_SET(offt, value) ((offt).QuadPart = (value))
#define FILE_OFFSET_TYPE_VALUE(offt) ((offt).QuadPart)
#define FILE_OFFSET_TYPE_ADDR(offt) (&(offt).QuadPart)
#if defined(_M_X64)
#define SIZE_T_TYPE_MOD	"I64"
#else
#define SIZE_T_TYPE_MOD ""
#endif

#else /* !_MSC_VER */

#define ALIGNED_ALLOC(p, sz, alignment) posix_memalign(&(p), alignment, sz)
#define ALIGNED_FREE(p) free(p)

  extern "C" { typedef void * (THREAD_START_ROUTINE)(void *); }
#define THREAD_PROC void *
  typedef pthread_t thread_id;

  typedef int	    FILE_HANDLE_TYPE;
#define INVALID_FILE_HANDLE_VALUE (-1)
#define FILE_POSITION_SET (SEEK_SET)
#define FILE_POSITION_CUR (SEEK_CUR)
#define FILE_POSITION_END (SEEK_END)
  typedef off_t   FILE_OFFSET_TYPE;
#ifdef __MACH__
#define FILE_OFFSET_TYPE_MOD            "ll"
#else
#define FILE_OFFSET_TYPE_MOD		"L"
#endif
#define FILE_OFFSET_TYPE_SET(offt, value) ((offt) = (value))
#define FILE_OFFSET_TYPE_VALUE(offt) ((long long)offt)
#define FILE_OFFSET_TYPE_ADDR(offt) ((long long*)&(offt))
#if defined(__x86_64__)
#define SIZE_T_TYPE_MOD			"l"
#else
#define SIZE_T_TYPE_MOD			""
#endif
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
        const W notfound = (W)(S << 3);
        std::size_t v = Mod67Position[((1 + ~(uint64_t)w) & (uint64_t)w) % 67];
        return (v != 64) ? v : notfound;
      }
    };

#if defined (__GNUC__)
#  if defined (__x86_64__)
    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        std::size_t v;
        const W notfound = 64;
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
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        unsigned v;
        const W notfound = 32;
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
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        std::size_t v = __builtin_ffsll((uint64_t)w);
        return v ? (v - 1) : 64;
      }
    };
    
    template <typename W> struct bitop<W, 4>
    {
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        std::size_t v = __builtin_ffs((uint32_t)w);
        return v ? (v - 1) : 32;
      }
    };
#  endif
#elif defined(_MSC_VER)

    template <typename W> struct bitop<W, 4>
    {
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        unsigned long v;
        return _BitScanForward(&v, (unsigned long)w) ? v : 32;
      }
    };

#  if defined(_M_X64)

    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        unsigned long v;
        return _BitScanForward64(&v, (unsigned __int64)w) ? v : 64;
      }
    };

#  elif defined(_M_IX86)

    template <typename W> struct bitop<W, 8>
    {
      static inline std::size_t PURE_DECL HEAVYUSE bsf(W w)
      {
        unsigned long v;
        return _BitScanForward( &v, (unsigned long) w ) ? v
             : _BitScanForward( &v, (unsigned long)(w >> 32) ) ? v + 32 : 64;
      }
    };

#  endif
#endif

    template <typename W>
    static inline std::size_t PURE_DECL HEAVYUSE bit_scan(W w, std::size_t from = 0)
    {
      return bitop<W>::bsf( w & ((from < (sizeof(W) << 3)) ? (~(W)0 << from): (W)0) );
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
        void reset(T* p = NULL) { if (p_) delete [] p_; p_ = p; }
        ~type() { if (p_) delete [] p_; }
      };
    };
#endif

    struct Sys
    {
#ifdef HAVE_TBB

      typedef tbb::tick_count TickCount;

#elif defined(_MSC_VER)
      class TickCount
      {
        ALIGN_DECL(16) LARGE_INTEGER m_data;

        TickCount(LARGE_INTEGER data) : m_data(data) {}

      public:
        static inline TickCount now()
        { LARGE_INTEGER data; ::QueryPerformanceCounter(&data); return data; }

	TickCount() { m_data.QuadPart = 0; }

        TickCount operator-(const TickCount& rhs) const
        {
          LARGE_INTEGER diff;
	  diff.QuadPart = m_data.QuadPart - rhs.m_data.QuadPart;
	  return diff;
        }

        double seconds() const
        {
	  LARGE_INTEGER freq;
	  ::QueryPerformanceFrequency(&freq);
	  return (double)m_data.QuadPart/(double)freq.QuadPart;
        }
      };
#elif defined(__MACH__)
      class TickCount
      {
        uint64_t m_data;

        TickCount(uint64_t data) : m_data(data) {}

      public:
        static inline TickCount now()
        { return ::mach_absolute_time(); }

        TickCount() : m_data(0) {}

        TickCount operator-(const TickCount& rhs) const
        {
          return m_data - rhs.m_data;
        }

        double seconds() const
        {
          ::mach_timebase_info_data_t tb;
          ::mach_timebase_info(&tb);
          double v = (double)m_data * tb.numer / tb_denom;
          return v * (1.0E-9);
        }
      };
#else
      class TickCount
      {
        struct timespec m_data;

        TickCount(struct timespec data) : m_data(data) {}

      public:
        static inline TickCount now()
        { struct timespec data; ::clock_gettime(CLOCK_REALTIME, &data); return data; }

        TickCount() { m_data.tv_sec = 0; m_data.tv_nsec = 0; }

        TickCount operator-(const TickCount& rhs) const
        {
          struct timespec data;
          data.tv_sec = m_data.tv_sec - rhs.m_data.tv_sec;
          if( m_data.tv_nsec >= rhs.m_data.tv_nsec )
            data.tv_nsec = m_data.tv_nsec - rhs.m_data.tv_nsec;
          else
          {
            data.tv_sec -= 1;
            data.tv_nsec = 1000000000 + m_data.tv_nsec - rhs.m_data.tv_nsec;
          }
          return data;
        }

        double seconds() const
        {
          return (double)m_data.tv_sec + (double)m_data.tv_nsec / 1.0E-9;
        }
      }; // TickCount
#endif

      class Semaphore
      {
        Semaphore( const Semaphore & );
        Semaphore & operator = ( const Semaphore & );
#if defined(_MSC_VER)
        HANDLE m_sem;
      public:
        Semaphore(int count = 0) 
        {
          m_sem = ::CreateSemaphore(NULL, (std::numeric_limits<int>::max)(), count, NULL);
        }
        ~Semaphore()
        {
          ::CloseHandle( m_sem );
        }
        bool wait()
        {
          return WAIT_OBJECT_0 == ::WaitForSingleObject( m_sem, INFINITE );
        }
        bool try_wait()
        {
          return WAIT_OBJECT_0 == ::WaitForSingleObject( m_sem, 0L );
        }
        bool post()
        {
          return ::ReleaseSemaphore(m_sem, 1, NULL) != 0;
        }

#elif defined(__MACH__)
        dispatch_semaphore_t m_sem;
      public:
        Semaphore( int count = 0 )
        {
          m_sem = ::dispatch_semaphore_create( count );
        }
        ~Semaphore()
        {
          ::dispatch_release( m_sem );
        }
        bool wait()
        {
          return 0 == ::dispatch_semaphore_wait( m_sem, DISPATCH_TIME_FOREVER );
        }
        bool try_wait()
        {
          return 0 == ::dispatch_semaphore_wait( m_sem, DISPATCH_TIME_NOW );
        }
        bool post()
        {
          ::dispatch_semaphore_signal( m_sem );
          return true;
        }
#else
        sem_t m_sem;
      public:
        Semaphore( int count = 0 ) 
        {
          ::sem_init( &m_sem, 0, count );
        }
        ~Semaphore()
        {
          ::sem_destroy( &m_sem );
        }
        bool wait()
        {
          return 0 == ::sem_wait( &m_sem );
        }
        bool try_wait()
        {
          return 0 == ::sem_trywait( &m_sem );
        }
        bool post()
        {
          return 0 == ::sem_post( &m_sem );
        }
#endif
      }; // Semaphore

#if defined(_MSC_VER)
      static inline void SchedYield() { ::SwitchToThread(); }
#else
      static inline void SchedYield() { ::sched_yield(); }
#endif
    }; // Sys

    struct NumData
    {
      union DigitPair
      {
        char     c[2];
        uint16_t u;
      };
      static ALIGN_DECL_DEFAULT DigitPair m_pairs[100]; /* string representations of "00" to "99" */
    };

#if (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))) || defined(_MSC_VER)
    class x86Data : public NumData {
      protected:
      union Log2 {
        struct {
          int32_t m_count;
          uint32_t m_threshold;
        };
        uint64_t m_qword;
      };
      static ALIGN_DECL_DEFAULT Log2 m_digits[32];
      static struct ConvBits
      { 
        uint16_t mul_10[8];
        uint16_t div_const[8];
        uint16_t shl_const[8];
        uint8_t  to_ascii[16];
      } ALIGN_DECL(64) cbits;
    };

    class UInt : public x86Data {
      public:
      static inline std::size_t PURE_DECL HEAVYUSE numDigits( uint32_t i )
      {
        std::size_t l = i;
#ifdef __GNUC__
  #ifdef __x86_64__
        __asm__ __volatile__ (
                "bsrl %k0, %k0          \n\t"
                "shlq $32, %q1          \n\t"
                "movq (%2,%0,8), %0     \n\t"
                "cmpq %0, %q1           \n\t"
                "seta %b1               \n\t"
                "addl %1, %k0           \n\t"
                : "+r" (l), "+r"(i)
                : "r"(m_digits)
                : "cc"
        );
        return l;
  #else
        __asm__ __volatile__ ( "bsrl %0, %0; " : "+r" (l) : : "cc" );
        const Log2& e = m_digits[l];
        return e.m_count + ( i > e.m_threshold );
  #endif
#else
        l = _BitScanReverse((DWORD*)&l, i) ? l : 0;
        const Log2& e = m_digits[l];
        return e.m_count + ( i > e.m_threshold );
#endif
      }

      // serializer for an integer with a known length
      static inline NOTHROW HEAVYUSE void generate(char* buf, uint32_t value, std::size_t len)
      {
#if defined(ENABLE_INT_SERIALIZER_BW)
        uint32_t u = value;
        switch(len) {
          default: u = (value * 1374389535ULL) >> 37; *(uint16_t*)(buf + 8) = m_pairs[value -= 100 * u].u; 
          case  8: value = (u * 1374389535ULL) >> 37; *(uint16_t*)(buf + 6) = m_pairs[u -= 100 * value].u; 
          case  6: u = (value * 1374389535ULL) >> 37; *(uint16_t*)(buf + 4) = m_pairs[value -= 100 * u].u;
          case  4: value = (u * 167773) >> 24; *(uint16_t*)(buf + 2) = m_pairs[u -= 100 * value].u;
          case  2: *(uint16_t*)(buf) = m_pairs[value].u;
          case  0: return;
          case  9: u = (value * 1374389535ULL) >> 37; *(uint16_t*)(buf + 7) = m_pairs[value -= 100 * u].u;
          case  7: value = (u * 1374389535ULL) >> 37; *(uint16_t*)(buf + 5) = m_pairs[u -= 100 * value].u;
          case  5: u = (value * 1374389535ULL) >> 37; *(uint16_t*)(buf + 3) = m_pairs[value -= 100 * u].u;
          case  3: value = (u * 167773) >> 24; *(uint16_t*)(buf + 1) = m_pairs[u -= 100 * value].u;
          case  1: *buf = value + 0x30;
        }
#elif defined(ENABLE_INT_SERIALIZER_BW_SHORT)
        std::size_t odd = len & 1;
        uint32_t u = value;
        switch(len >> 1) { 
          default: u = (value * 1374389535ULL) >> 37; *(uint16_t*)(buffer + 8) = m_pairs[value - 100 * u].u; 
          case  4: value = (u * 1374389535ULL) >> 37; *(uint16_t*)(buffer + 6 + odd) = m_pairs[u - 100 * value].u; 
          case  3: u = (value * 1374389535ULL) >> 37; *(uint16_t*)(buffer + 4 + odd) = m_pairs[value - 100 * u].u;
          case  2: value = (u * 1374389535ULL) >> 37; *(uint16_t*)(buffer + 2 + odd) = m_pairs[u - 100 * value].u;
          case  1: u = (value * 6554) >> 16; *(uint16_t*)buffer = m_pairs[odd ? u : value].u; value -= 10 * u; len = -1;
          case  0: buffer[odd - len] = value + 0x30;
        }
#else
  #if defined(__GNUC__) && defined(__x86_64__)
        uint16_t w;
        uint32_t u;
        __asm__ __volatile__ (
          "leaq T%=(%%rip), %q1           \n\t"
          "addq (%q1,%3,8), %q1	  	  \n\t"
          "jmp *%q1	                  \n\t"
          "T%=: .quad L0%=-T%=        \n\t"
          "     .quad L1%=-T%=        \n\t"
          "     .quad L2%=-T%=        \n\t"
          "     .quad L3%=-T%=        \n\t"
          "     .quad L4%=-T%=        \n\t"
          "     .quad L5%=-T%=        \n\t"
          "     .quad L6%=-T%=        \n\t"
          "     .quad L7%=-T%=        \n\t"
          "     .quad L8%=-T%=        \n\t"
          "     .quad L9%=-T%=        \n\t"
          "     .quad L10%=-T%=       \n\t"
          "L10%=:                     \n\t"
          " imulq $1441151881, %q0, %q1   \n\t"
          " shrq $57, %q1                 \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $100000000, %1, %1      \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, (%4)                \n\t"
          "L8%=:                      \n\t"
          " imulq $1125899907, %q0, %q1   \n\t"
          " shrq $50, %q1                 \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $1000000, %1, %1        \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, -8(%4,%3)           \n\t"
          "L6%=:                      \n\t"
          " imulq $429497, %q0, %q1       \n\t"
          " shrq $32, %q1                 \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $10000, %1, %1          \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, -6(%4,%3)           \n\t"
          "L4%=:                      \n\t"
          " imull $167773, %0, %1         \n\t"
          " shrl $24, %1                  \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $100, %1, %1            \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, -4(%4,%3)           \n\t"
          "L2%=:                      \n\t"
          " movw (%5,%q0,2), %w2          \n\t"
          " movw %w2, -2(%4,%3)           \n\t"
          "L0%=: jmp 1f               \n\t"
          "L9%=:                      \n\t"
          " imulq $1801439851, %q0, %q1   \n\t"
          " shrq $54, %q1                 \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $10000000, %1, %1       \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, (%4)                \n\t"
          "L7%=:                      \n\t"
          " imulq $43980466, %q0, %q1     \n\t"
          " shrq $42, %q1                 \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $100000, %1, %1         \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, -7(%4,%3)           \n\t"
          "L5%=:                      \n\t"
          " imulq $268436, %q0, %q1       \n\t"
          " shrq $28, %q1                 \n\t"
          " movw (%5,%q1,2), %w2          \n\t"
          " imull $1000, %1, %1           \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, -5(%4,%3)           \n\t"
          "L3%=:                      \n\t"
          " imull $6554, %0, %1           \n\t"
          " shrl $15, %1                  \n\t"
          " andb $254, %b1                \n\t"
          " movw (%5,%q1), %w2            \n\t"
          " leal (%1,%1,4), %1            \n\t"
          " subl %1, %0                   \n\t"
          " movw %w2, -3(%4,%3)           \n\t"
          "L1%=:                      \n\t"
          " addl $48, %0                  \n\t"
          " movb %b0, -1(%4,%3)           \n\t"
          "1:                         \n\t"
          : "+r"(value), "=&r"(u), "=&r"(w)
          : "r"(len), "r"(buf), "r"(m_pairs)
          : "memory", "cc"
        ); 
  #elif defined(_MSC_VER) && defined(_M_X64)
        uint32_t u;
        switch(len) {
          default: u = (value * 1441151881ULL) >> 57; *(uint16_t*)(buf) = m_pairs[u].u; value -= u * 100000000;
          case  8: u = (value * 1125899907ULL) >> 50; *(uint16_t*)(buf + len - 8) = m_pairs[u].u; value -= u * 1000000;
          case  6: u = (value * 429497ULL) >> 32; *(uint16_t*)(buf + len - 6) = m_pairs[u].u; value -= u * 10000;
          case  4: u = (value * 167773) >> 24; *(uint16_t*)(buf + len - 4) = m_pairs[u].u; value -= u * 100;
          case  2: *(uint16_t*)(buf + len - 2) = m_pairs[value].u;
          case  0: return;
          case  9: u = (value * 1801439851ULL) >> 54; *(uint16_t*)(buf) = m_pairs[u].u; value -= u * 10000000; 
          case  7: u = (value * 43980466ULL) >> 42; *(uint16_t*)(buf + len - 7) = m_pairs[u].u; value -= u * 100000; 
          case  5: u = (value * 268436ULL) >> 28; *(uint16_t*)(buf + len - 5) = m_pairs[u].u; value -= u * 1000;
          case  3: u = (value * 6554) >> 16; *(uint16_t*)(buf + len - 3) = m_pairs[u].u; value -= u * 10;
          case  1: *(buf + len - 1) = value + 0x30;
        }
  #else
        uint32_t u;
        switch(len) {
          default: u = value / 100000000; *(uint16_t*)(buf) = m_pairs[u].u; value -= u * 100000000;
          case  8: u = value / 1000000; *(uint16_t*)(buf + len - 8) = m_pairs[u].u; value -= u * 1000000;
          case  6: u = value / 10000; *(uint16_t*)(buf + len - 6) = m_pairs[u].u; value -= u * 10000;
          case  4: u = value / 100; *(uint16_t*)(buf + len - 4) = m_pairs[u].u; value -= u * 100;
          case  2: *(uint16_t*)(buf + len - 2) = m_pairs[value].u;
          case  0: return;
          case  9: u = value / 10000000; *(uint16_t*)(buf) = m_pairs[u].u; value -= u * 10000000; 
          case  7: u = value / 100000; *(uint16_t*)(buf + len - 7) = m_pairs[u].u; value -= u * 100000; 
          case  5: u = value / 1000; *(uint16_t*)(buf + len - 5) = m_pairs[u].u; value -= u * 1000;
          case  3: u = value / 10; *(uint16_t*)(buf + len - 3) = m_pairs[u].u; value -= u * 10;
          case  1: *(buf + len - 1) = value + 0x30;
        }
  #endif
#endif
      }

      // serializer that calculates length, buffer must have space for any value including terminating '\0'
      static inline unsigned NOTHROW HEAVYUSE generate(char* buf, uint32_t value)
      {
        uint32_t x, y, l = 0;

        x = (value * 1374389535ULL) >> 37;
        y = value;
        if (x) {
          uint32_t div_10000 = 0xd1b71759, mul_10000 = 55536;
          __m128i z, m, a, o;
          y -= 100 * x;
          z = _mm_cvtsi32_si128(x);
          m = _mm_load_si128((__m128i*)cbits.mul_10);
          o = _mm_mul_epu32( z, _mm_cvtsi32_si128(div_10000));
          z = _mm_add_epi32( z, _mm_mul_epu32( _mm_cvtsi32_si128(mul_10000), _mm_srli_epi64( o, 45) ) );
          z = _mm_slli_epi64( _mm_shuffle_epi32( _mm_unpacklo_epi16(z, z), 5 ), 2 );
          a = _mm_load_si128((__m128i*)cbits.to_ascii);
          z = _mm_mulhi_epu16( _mm_mulhi_epu16( z, *(__m128i*)cbits.div_const ), *(__m128i*)cbits.shl_const );
          z = _mm_sub_epi16( z, _mm_slli_epi64( _mm_mullo_epi16( m, z ), 16 ) );
          z = _mm_add_epi8( _mm_packus_epi16( z, _mm_xor_si128(o, o) ), a );
#if defined(_MSC_VER)
          _BitScanForward( (DWORD*)&x, ~_mm_movemask_epi8( _mm_cmpeq_epi8( a, z ) ) );
#else
          x = __builtin_ctz( ~_mm_movemask_epi8( _mm_cmpeq_epi8( a, z ) ) );
#endif
          l = 8 - x;
#if defined(__x86_84__) || defined(_M_X64)
          uint64_t q = (uint64_t)_mm_cvtsi128_si64(z) >> (x * 8);
          *(uint64_t*)buf = q;
#else
          z = _mm_srli_epi64( z, x * 8 );
          _mm_storel_epi64( (__m128i*)buf, z );
#endif
          buf += l;
          x = 1;
        }
        value = (y * 6554) >> 16;
        l += 1 + (x | (value != 0));
        *(uint32_t*)buf = 0x30 + ((l > 1) ? ((0x30 + y - value * 10) << 8) + value : y);
        return l;
      }

      static inline short PURE_DECL HEAVYUSE checkSum(uint32_t n)
      {
        short csum = (int)'0' + n % 10;
#ifdef INTEL_COMPILER
#pragma loop count min(0) max(9)
#endif
        for( n /= 10; n; n /= 10 )
          csum += (int)'0' + n % 10;
        return csum;
      }
    };

#else // generics

    class UInt : public NumData {
      public:
      static inline std::size_t PURE_DECL HEAVYUSE numDigits( uint32_t i )
      {
        if (i >= 100)
           if (i < 1000000)
              if (i < 10000)
                 return 3 + (i >= 1000);
              else
                 return 5 + (i >= 100000);
           else
              if (x < 100000000)
                 return 7 + (i >= 10000000);
              else
                 return 9 + (i >= 1000000000);
        return 1 + (i > 9);
      }
      // serializer for an integer of a known length
      static inline NOTHROW HEAVYUSE void generate(char* buf, uint32_t value, std::size_t len)
      {
        for (int n = 1; len > n; n++)
        {
          uint32_t u = value / 10;
          buf[len - n] = '0' + (value - u * 10);
          value = u;
        }
        *buf = '0' + value;
      }
      // serializer that calculates length, buffer must have space for any value including terminating '\0'
      static inline unsigned NOTHROW HEAVYUSE generate(char* buf, uint32_t value)
      {
        uint32_t u = value / 10;
        uint32_t c0 = value - u * 10;
        char* p = buf;
        if (u) {
           value = u / 10;
           uint32_t c1 = u - value * 10;
           if (value) {
             u = value / 10;
             uint32_t c2 = value - u * 10;
             if (u) {
               value = u / 10;
               uint32_t c3 = u - value * 10;
               if (value) {
                 u = value / 10;
                 uint32_t c4 = value - u * 10;
                 if (u) {
                   value = u / 10;
                   uint32_t c5 = u - value * 10;
                   if (value) {
                     u = value / 10;
                     uint32_t c6 = value - u * 10;
                     if (u) {
                       value = u / 10;
                       uint32_t c7 = u - value * 10;
                       if (value) {
                         u = value / 10;
                         uint32_t c8 = value - u * 10;
                         if (u) {
                           *p++ = '0' + u;
                         }
                         *p++ = '0' + c8;
                       }
                       *p++ = '0' + c7;
                     }
                     *p++ = '0' + c6;
                   }
                   *p++ = '0' + c5;
                 }
                 *p++ = '0' + c4;
               }
               *p++ = '0' + c3;
             }
             *p++ = '0' + c2;
           }
           *p++ = '0' + c1;
        }
        *p++ = '0' + c0;
        *p = '\0';
        return p - buf;
      }
      static inline short PURE_DECL HEAVYUSE checkSum(int n)
      {
        short csum = (int)'0' + n % 10;
        for( n /= 10; n; n /= 10 )
          csum += (int)'0' + n % 10;
        return csum;
      }
    };
#endif

    class Int {
      public:
      static inline std::size_t PURE_DECL HEAVYUSE numDigits( int32_t i )
      {
        return UInt::numDigits( (i < 0) ? uint32_t(~i) + 1 : i ) + (i < 0);
      }

      static inline unsigned NOTHROW HEAVYUSE generate(char* buf, int32_t value)
      {
        *buf = '-';
        bool neg = value < 0;
        return UInt::generate(buf + neg, neg ? uint32_t(~value) + 1 : value ) + neg;
      }
    };

    class ULong {
#if (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))) || defined(_MSC_VER)
      struct Log2 {
        uint64_t m_threshold;
        int64_t m_count;
      };
      static ALIGN_DECL_DEFAULT Log2 m_digits[64];
      public:
      static inline std::size_t PURE_DECL HEAVYUSE numDigits( uint64_t i )
      {
        uint64_t log2 = i;
#ifdef __x86_64__
        __asm__ __volatile__ ( "bsrq %0, %0; " : "+r" (log2) : : "cc" );
#elif defined __i386__
        log2 = log2 ? __builtin_clzll(log2) : 0;
#elif defined _M_X64
        log2 = _BitScanReverse64(&log2, log2) ? log2 : 0;
#else // _M_IX86
        uint32_t v;
        log2 = _BitScanReverse( (DWORD*)&v, (unsigned long)(log2 >> 32)) ? v + 32
             : _BitScanReverse( (DWORD*)&v, (unsigned long)(log2) ) ? v : 0;
#endif
        const Log2& e = m_digits[log2];
        return (std::size_t)(e.m_count + ( i > e.m_threshold ));
      }
#else
      public:
      static inline std::size_t PURE_DECL HEAVYUSE numDigits( uint64_t i )
      {
        if (i >= 100)
           if (i < 10000000000ULL)
             if (i < 1000000)
                if (i < 10000)
                   return 3 + (i >= 1000);
                else
                   return 5 + (i >= 100000);
             else
                if (x < 100000000)
                   return 7 + (i >= 10000000);
                else
                   return 9 + (i >= 1000000000);
           else
             if (i >= 1000000000000ULL)
               if (i < 10000000000000000ULL)
                  if (i < 100000000000000ULL)
                     return 13 + (i >= 10000000000000ULL);
                  else
                     return 15 + (i >= 1000000000000000ULL);
               else
                  if (x < 1000000000000000000ULL)
                     return 17 + (i >= 100000000000000000ULL);
                  else
                     return 19 + (i >= 10000000000000000000ULL);
             else
               return 11 + (i >= 100000000000ULL);
        return 1 + (i > 9);
      }
#endif
      static inline std::size_t PURE_DECL HEAVYUSE numFractionDigits( uint64_t v )
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

    struct Memory
    {
      template <typename T> static inline void shift_up_from(T* src, std::size_t count)
      {
        if (LIKELY(count < 8) ) while (count) { std::size_t c = count--; src[c] = src[count]; }
        else ::memmove(src + 1, src, count * sizeof(T));
      }
      template <typename T> static inline void shift_down_to(T* src, std::size_t count)
      {
        if (LIKELY(count < 8) ) while (count--) { T* dst = src++; *dst = *src; }
        else ::memmove(src, src + 1, count * sizeof(T));
      }
    };

    struct Char
    {
      static inline bool isspace( char x ) { return x == ' '; }
      static inline bool isdigit( char x ) { return (unsigned int)( x - '0' ) < 10; }
      static inline char toupper( char x ) { return ((x) >= 'a' && ((x) <= 'z') ? ((x)-('a'-'A')):(x)); }
    };

    struct CharBuffer
    {
      class reverse_iterator : public std::iterator<std::random_access_iterator_tag, char>
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
          bool operator <( reverse_iterator it) const
          { return m_p > it.m_p; }
          bool operator >( reverse_iterator it) const
          { return m_p < it.m_p; }
          difference_type operator-( reverse_iterator it) const
          { return it.m_p - m_p; }
      };

      static inline const char* memmem(const char* buf, std::size_t bsz,
                                       const char* str, std::size_t ssz)
      {
#if defined(__GNUC__)
	return (const char*)::memmem(buf, bsz, str, ssz);
#else
        if (bsz >= ssz)
        {
          const char* end = buf + bsz - ssz + 1;
		  char c = str[0];
          while (LIKELY(NULL != (buf = (const char*)::memchr (buf, c, end - buf))))
          {
            if (0 == ::memcmp (buf, str, ssz)) 
              return buf; 
            if (++buf >= end)
              break;
          }
        }
        return 0; 
#endif
      }

      static inline int32_t PURE_DECL HEAVYUSE checkSum(const char* p, int size)
      {
        int32_t x = 0;
#if defined(__GNUC__) && defined(__x86_64__)
        int32_t n = size;
        const uint8_t* pu = (const uint8_t*)p;
        const uint64_t mask = 0x0101010101010101ULL;

        if (n >= 8)
              __asm__ __volatile (
              " cmpl $16, %2;                      \n\t"
#ifdef __AVX__
              " vpxor %%xmm0, %%xmm0, %%xmm0;      \n\t"
              " vpxor %%xmm2, %%xmm2, %%xmm2;      \n\t"
              " jl 1f;                \n\t"
              " vmovq %3, %%xmm1;                  \n\t"
              " vpunpcklbw %%xmm0, %%xmm1, %%xmm1; \n\t"
              " .p2align 4,,10;                    \n\t"
              " .p2align 3;                        \n\t"
              // loop over 16-byte blocks
              "0:;                    \n\t"
              " vlddqu (%1), %%xmm4;               \n\t"
              " addl $-16, %2;                     \n\t"
              " vpunpcklbw %%xmm2, %%xmm4, %%xmm3; \n\t"
              " addq $16, %1;                      \n\t"
              " vpmaddwd %%xmm1, %%xmm3, %%xmm3;   \n\t"
              " cmpl $16, %2;                      \n\t"
              " vpunpckhbw %%xmm2, %%xmm4, %%xmm4; \n\t"
              " vpaddd %%xmm3, %%xmm0, %%xmm0;     \n\t"
              " vpmaddwd %%xmm1, %%xmm4, %%xmm4;   \n\t"
              " vpaddd %%xmm4, %%xmm0, %%xmm0;     \n\t"
              " jge 0b;               \n\t"
              " cmpl $4, %2;                       \n\t"
              " jl 2f;                \n\t"
              " .p2align 4,,10;                    \n\t"
              " .p2align 3;                        \n\t"
              // loop over 4-byte blocks
              "1:                     \n\t"
              " vmovd (%1), %%xmm3;                \n\t"
              " addq $4, %1;                       \n\t"
              " vpunpcklbw %%xmm2, %%xmm3, %%xmm3; \n\t"
              " addl $-4, %2;                      \n\t"
              " vpunpcklwd %%xmm2, %%xmm3, %%xmm3; \n\t"
              " cmpl $4, %2;                       \n\t"
              " vpaddd %%xmm3, %%xmm0, %%xmm0;     \n\t"
              " jge 1b;               \n\t"
              // accumulate
              "2:;                    \n\t"
              " vpsrldq $8, %%xmm0, %%xmm1;        \n\t"
              " vpaddd %%xmm1, %%xmm0, %%xmm0;     \n\t"
              " vpsrldq $4, %%xmm0, %%xmm2;        \n\t"
              " vpaddd %%xmm2, %%xmm0, %%xmm0;     \n\t"
              " vmovd %%xmm0, %0;                  \n\t"
#else
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
#endif
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
        for (; size > 0; p++, size--)
          x += *(unsigned char*)p;
#endif
        return x;
      }

      template <std::size_t S> union Fixed
      {
        char data[S];

        /// sets contents to s
        bool set(const char* s, std::size_t size, std::size_t at = 0, std::size_t narrow = 1) {
	  if (at + size <= (S - narrow)) {
	    for (char* p = data + at; size; size--) *p++ = *s++;
	    return true;
          }
          return false;
        }
      };

      /// Scans f for character v, returns array index less than S on success
      /// or a value >= S on failure.
      template <std::size_t S>
      static inline std::size_t find(int v, const Fixed<S>& f)
      {
        const char* p = (const char*)::memchr(f.data, v, S);
        return p ? p - f.data : S;
      }

      /// Locates a substring f.
      template <std::size_t S>
      static inline const char* find(const Fixed<S>& f, const char* p, std::size_t size)
      {
        return CharBuffer::memmem( p, size, f.data, S );
      }
    }; // CharBuffer

    template <> union CharBuffer::Fixed<2>
    {
      typedef uint16_t value_type;

      char data[2];
      value_type value;
    };

    template <> union CharBuffer::Fixed<4>
    {
      typedef uint32_t value_type;

      char data[4];
      value_type value;
    };

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)

    template <>
    inline const char* CharBuffer::find<2>(const CharBuffer::Fixed<2>& f,
                                           const char* p, std::size_t size)
    {
      for (; size >= 2; size--, p++)
        if (*(const uint16_t*)p == f.value )
          return p;
      return NULL;
    }

    template <>
    inline const char* CharBuffer::find<4>(const CharBuffer::Fixed<4>& f,
                                           const char* p, std::size_t size)
    {
      for (; size >= 4; size--, p++)
        if (*(const uint32_t*)p == f.value )
          return p;
      return NULL;
    }

#endif

    template <> union CharBuffer::Fixed<8>
    {
      typedef uint64_t value_type;

      char data[8];
      value_type value;
    };

#if defined(__GNUC__) && defined(__x86_64__)

    template<>
    inline std::size_t PURE_DECL CharBuffer::find<8>(int v, const CharBuffer::Fixed<8>& f)
    {
      unsigned at = 0;
      __asm__ __volatile (
#if defined(__AVX__)
        "vmovd %1, %%xmm1                  \n\t"
        "vpunpcklbw %%xmm1, %%xmm1, %%xmm1 \n\t"
        "vmovq %2, %%xmm0                  \n\t"
        "vpshuflw $0, %%xmm1, %%xmm1       \n\t"
        "movl $8, %1                       \n\t"
        "vpcmpeqb %%xmm1, %%xmm0, %%xmm0   \n\t"
        "vpmovmskb %%xmm0, %0              \n\t"
        "bsf %0, %0                        \n\t"
        "cmovz %1, %0                      \n\t"
        : "=a" (at), "+r" (v)
        : "r" (f.value)
        : "xmm0", "xmm1"
#else
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
        : "xmm0", "xmm1"
#endif
      );
      return at;
    }
#endif 

#if defined(_MSC_VER) || (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)))

    // specializations for use with short strings
    template <> union CharBuffer::Fixed<15>
    {
      char data[15];

      inline bool set(const char* s, std::size_t len, std::size_t pos = 0, std::size_t narrow = 1) {
        if (LIKELY(len + pos <= (15 - narrow))) {
	  if (len == 1) {
	    data[pos] = s[0];
	  } else if (LIKELY(len > 0)) {
	    char* p = data + pos;
	    if (len <= 4) {
		*(uint16_t*)p= *(uint16_t*)s; 
		*(uint16_t*)(p + len - 2) = *(uint16_t*)(s + len - 2);
	    } else if (len <= 8) {
		*(uint32_t*)p= *(uint32_t*)s;
		*(uint32_t*)(p + len - 4) = *(uint32_t*)(s + len - 4);
	    } else {
		*(uint64_t*)p= *(uint64_t*)s;
		*(uint64_t*)(p + len - 8) = *(uint64_t*)(s + len - 8);
	    } 
	  }
          return true;
        }
	return false;
      }
    };

    template <> union CharBuffer::Fixed<23>
    {
      char data[23];

      inline bool set(const char* s, std::size_t len, std::size_t pos = 0, std::size_t narrow = 1) {
        if (LIKELY(len + pos <= (23 - narrow))) {
	  if (len == 1) {
	    data[pos] = s[0];
	  } else if (LIKELY(len > 0)) {
	    char* p = data + pos;
	    if (len <= 4) {
		*(uint16_t*)p= *(uint16_t*)s; 
		*(uint16_t*)(p + len - 2) = *(uint16_t*)(s + len - 2);
	    } else if (len <= 8) {
		*(uint32_t*)p= *(uint32_t*)s;
		*(uint32_t*)(p + len - 4) = *(uint32_t*)(s + len - 4);
	    } else if (len <= 16) {
		*(uint64_t*)p= *(uint64_t*)s;
		*(uint64_t*)(p + len - 8) = *(uint64_t*)(s + len - 8);
	    } else {
		_mm_storeu_si128((__m128i*)p, _mm_loadu_si128((__m128i*)s));
		*(uint64_t*)(p + len - 8) = *(uint64_t*)(s + len - 8);
	    }
	  }
          return true;
        }
	return false;
      }
    };
#endif
  
    class Tag
#if defined(_MSC_VER) || (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)))
              : public x86Data
#endif
    {
      /* NOTE: Allow for tag value range of 1-99999999. */

      static const std::size_t MaxValueSize = 8; // tag length limit

      static inline void NOTHROW HEAVYUSE generate(char* buf, uint32_t x, int digits)
      {
#if defined(_MSC_VER) || (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)))
        uint32_t v;

        if( LIKELY(x <= 999) )
        {
          if ( LIKELY(x > 9) )
          {
              v = (x * 6554) >> 16;
              *(uint16_t*)buf = m_pairs[(digits & 1) ? v : x].u;
              x -= v * 10;
          }
          buf[digits - 1] = x + 0x30; 
        }
        else
        {
          uint32_t div_10000 = 0xd1b71759, mul_10000 = 55536;

            /* Based on utoa32_sse_2 routine by Wojciech Mula
             * http://wm.ite.pl/articles/sse-itoa.html
             */
          __m128i z, m, a, o;
          z = _mm_cvtsi32_si128(x);
          m = _mm_load_si128((__m128i*)cbits.mul_10);
          o = _mm_mul_epu32( z, _mm_cvtsi32_si128(div_10000));
          z = _mm_add_epi32( z, _mm_mul_epu32( _mm_cvtsi32_si128(mul_10000), _mm_srli_epi64( o, 45) ) );
          z = _mm_slli_epi64( _mm_shuffle_epi32( _mm_unpacklo_epi16(z, z), 5 ), 2 );
          a = _mm_load_si128((__m128i*)cbits.to_ascii);
          z = _mm_mulhi_epu16( _mm_mulhi_epu16( z, *(__m128i*)cbits.div_const ), *(__m128i*)cbits.shl_const );
          z = _mm_sub_epi16( z, _mm_slli_epi64( _mm_mullo_epi16( m, z ), 16 ) );
          z = _mm_add_epi8( _mm_packus_epi16( z, _mm_xor_si128(o, o) ), a );
#if defined(__x86_64__) || defined(_M_X64)
          uint64_t packed = (uint64_t)_mm_cvtsi128_si64(z) >> ((8 - digits) * 8);
          if (LIKELY(digits <= 4))
            *(uint32_t*)buf = (uint32_t)packed;
          else
            *(uint64_t*)buf = packed;	// can overwrite space reserved for '=', '\001' and '\000'
#else
          z = _mm_srli_epi64( z, (8 - digits) * 8 );
          if (LIKELY(digits <= 4))
            *(uint32_t*)buf = _mm_cvtsi128_si32( z );
          else
            _mm_storel_epi64( (__m128i*)buf, z );
#endif
        }
#else
        UInt::generate(buf, x, digits);
#endif
      }

    public:

      struct Identity : public std::unary_function<int, std::size_t> {
        inline std::size_t operator()(int tag) const { return tag; }
      };

      template <typename S> static void append(S& result, int value, int digits)
      {
        char buf[MaxValueSize];
        generate(buf, (uint32_t)value, digits);
	result.append(buf, digits);
      }

      template <typename S> static void set(S& result, int value, int digits)
      {
        char buf[MaxValueSize];
        generate(buf, (uint32_t)value, digits);
	result.assign(buf, digits);
      }

      static void NOTHROW HEAVYUSE write(char* result, int value, int digits)
      {
	generate(result, (uint32_t)value, digits);
      }

      static inline const char NOTHROW_PRE * PURE_DECL HEAVYUSE NOTHROW_POST delimit(const char* p, unsigned len)
      {
#if defined(__GNUC__) && defined(__x86_64__)
        if ( LIKELY(len > 1) )
        {
          uint64_t v = 0x3d3d3d3d3d3d3d3dULL;
          uint64_t pos = ~(uintptr_t)p;
          __asm__ __volatile__ (
#ifdef __AVX__
                "vmovq %0, %%xmm0                       \n\t"
                "vmovq %4, %%xmm1                       \n\t"
                "vpunpcklqdq %%xmm0, %%xmm0, %%xmm0     \n\t"
                "vpcmpeqb %%xmm1, %%xmm0, %%xmm0        \n\t"
                "movl %3, %%ecx                         \n\t"
                "vpmovmskb %%xmm0, %0                   \n\t"
#else
                "movq %0, %%xmm0                        \n\t"
                "movq %4, %%xmm1                        \n\t"
                "punpcklqdq %%xmm0, %%xmm0              \n\t"
                "pcmpeqb %%xmm1, %%xmm0                 \n\t"
                "movl %3, %%ecx                         \n\t"
                "pmovmskb %%xmm0, %0                    \n\t"
#endif
                "add $1, %2                             \n\t"
                "bsf %0, %0                             \n\t"
                "cmovnz %0, %%rcx                       \n\t"
                "cmpl %3, %%ecx                         \n\t"
                "cmovl %%rcx, %1                        \n\t"
                : "+r"(v), "+r"(pos), "+r"(p)
                : "r"(len), "m"(*(p + 1))
                : "cc", "rcx", "xmm0", "xmm1"
          );
          return p + pos;
        }
        return NULL;
#else
        switch (len)
        {
          default: if (LIKELY(*p != '=')) { p++;
                   if (LIKELY(*p != '=')) { p++;
          case 8:  if (LIKELY(*p != '=')) { p++;
          case 7:  if (LIKELY(*p != '=')) { p++;
          case 6:  if (LIKELY(*p != '=')) { p++;
          case 5:  if (LIKELY(*p != '=')) { p++;
          case 4:  if (LIKELY(*p != '=')) { p++;
          case 3:  if (LIKELY(*p != '=')) { p++;
          case 2:  if (LIKELY(*p != '=')) { p++;
                   if (LIKELY(*p != '=')) { p++;
          case 1:
          case 0:  return NULL;
                } } } } } } } } } }
        }
        return p;
#endif
      }

      // store correct accumulated value even in case of a stop at a non-digit character
      static inline bool HEAVYUSE NOTHROW parse(const char* b, const char* e, int& rv, int& rx)
      {
	bool valid = false;
	unsigned a, c, s = *b == '-';
        int v = 0, x = s ? '-' : 0;
	switch (e - (b += s))
	{
          case 8: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = a; x = c; 
          case 7: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c; 
          case 6: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c; 
          case 5: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c; 
          case 4: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c; 
          case 3: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c; 
          case 2: a = (c = *b++) - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c; 
          case 1: a = (c = *b)   - '0'; if (LIKELY(a < 10)) { v = v * 10 + a; x += c;
                  valid = true; } } } } } } } }
	}
        rv = LIKELY(!s) ? v : -v; rx = x;
        return valid;
      }

      static inline char NOTHROW length(int field)
      {
        return Util::UInt::numDigits((unsigned)field) + 1;
      }

      static inline short NOTHROW checkSum(int field)
      {
        return Util::UInt::checkSum((unsigned)field) + (short)'=';
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
        std::size_t v = detail::bit_scan(m_bits[i], ++b);
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
            word_type w = m_bits[src];
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
            word_type w = m_bits[src];
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
        return detail::bit_scan(m_bits, ++b);
      }

      inline BitSet NOTHROW_PRE & NOTHROW_POST operator <<=(int n) { m_bits <<= n; return *this; }

      inline BitSet NOTHROW_PRE & NOTHROW_POST operator >>=(int n) { m_bits >>= n; return *this; }

      std::size_t size() const { return sizeof(word_type) << 3; }
    };

    template <typename Key, typename T, std::size_t N> class PtrCache {
      BitSet<N> m_mask;
      Key m_key[N];
      T* m_p[N];

    public:

      static const std::size_t Alignment = 1;

      PtrCache() : m_mask(true) {}

      static inline std::size_t capacity()
      { return N; }

      void clear()
      { m_mask.set(); }
  
      T* lookup(const Key& key)
      {
        for(std::size_t i = 0; i < N; i++)
          if (key == m_key[i] && !m_mask[i])
            return m_p[i];
        return NULL;
      }
      std::size_t insert(const Key& key, T* p)
      {
        std::size_t i = m_mask._Find_first();
        if (i != N)
        {
          m_key[i] = key;
          m_p[i] = p;
          m_mask.reset(i);
        }
        return i;
      }
      T* erase(const Key& key)
      {
        for (std::size_t i = 0; i < N; i++)
        {
          if (key == m_key[i] && !m_mask[i]) 
          {
            m_mask.set(i);
            return m_p[i];
          }
        }
        return false;
      }

      T* evict(std::size_t pos)
      { return (pos < N) ? m_mask.set(pos), m_p[pos] : NULL; }
    };

#if defined(__GNUC__) && defined(__x86_64__)

    namespace detail
    {
      template <std::size_t Size> struct int32_key_holder
      {
        union
        {
          __m128 m_simd[Size >> 2];
          struct
          {
            int32_t m_key[Size + 1];
            uint32_t m_mask;
          };
        } __attribute__((packed));
        int32_key_holder() : m_mask(((uint32_t)1 << Size) - 1) {}

        template <typename F> F* lookup_pos(uint32_t mask, F* m_p[Size + 1])
        {
          F* p = NULL;
          __asm__ __volatile__ (
          " bsf %1, %1;               \n\t"
          " cmovnz (%2,%q1,8), %0;    \n\t"
          : "+a"(p), "+r" (mask)
          : "r" (m_p)
          : "cc"
          );
          return p;
        }
        template <typename F> std::size_t insert_key(int32_t key, F* p, F* m_p[Size + 1]) {
          uint32_t i = Size;
          __asm__ __volatile__ (
          " bsf %1, %%edx;            \n\t"
          " cmovnz %%edx, %0;         \n\t"
          " btr %0, %1;               \n\t"
          : "+r"(i), "+m"(m_mask)
          : 
          : "edx", "cc"
          );
          m_key[i] = key;
          m_p[i] = p;
          return i;
        }
        template <typename F> F* erase_pos(uint32_t mask, F* m_p[Size + 1]) {
	  int32_t tmp;
          F* p = NULL;
          __asm__ __volatile__ (
          " bsf %0, %0;               \n\t"
          " cmovnz (%4,%q0,8), %3;    \n\t"
          " setnz %b1;                \n\t"
          " movzbl %b1, %1;           \n\t"
          " shl %b0, %1;              \n\t"
          " or %1, %2;                \n\t"
          : "+c"(mask), "=r" (tmp), "+m"(m_mask), "+r" (p)
          : "r" (m_p)
          : "cc"
          );
          return p;
        }
        template <typename F> F* evict_pos(std::size_t pos, F* m_p[Size + 1]) {
          std::size_t tmp = Size;
          F* p = NULL;
          __asm__ __volatile__ (
          " bts %2, %0                \n\t"
          " cmovc %4, %2              \n\t"
          " cmp %2, %4                \n\t"
          " cmovg (%3,%2,8), %1       \n\t"
          : "+m"(m_mask), "+r"(p)
          : "r"(pos), "r"(m_p), "r"(tmp)
          : "cc"
          );
          return p;
        }
      };

      template <std::size_t Size> struct int32_keys;

      template <> struct int32_keys<8> : public int32_key_holder<8>
      {
        inline uint32_t keymask(int32_t key)
        {
          uint32_t mask;
          __asm__ __volatile__ (
#ifdef __AVX__
          " vmovd   %1, %%xmm2;       \n\t"
          " vshufps $0, %%xmm2, %%xmm2, %%xmm2 \n\t"
          " vpcmpeqd %2, %%xmm2, %%xmm1; \n\t"
          " mov %4, %0;               \n\t"
          " vpcmpeqd %3, %%xmm2, %%xmm2; \n\t"
          " not %0;                   \n\t"
          " and $255, %0;             \n\t"
          " vpackssdw %%xmm2, %%xmm1, %%xmm1;  \n\t"
          " vpacksswb %%xmm1, %%xmm1, %%xmm1;  \n\t"
          " vpmovmskb %%xmm1, %1;     \n\t"
#else
          " movdqa %2, %%xmm1;        \n\t"
          " movd   %1, %%xmm2;        \n\t"
          " shufps $0, %%xmm2, %%xmm2 \n\t"
          " pcmpeqd %%xmm2, %%xmm1;   \n\t"
          " pcmpeqd %3, %%xmm2;       \n\t"
          " mov %4, %0;               \n\t"
          " not %0;                   \n\t"
          " and $255, %0;             \n\t"
          " packssdw %%xmm2, %%xmm1;  \n\t"
          " packsswb %%xmm1, %%xmm1;  \n\t"
          " pmovmskb %%xmm1, %1;      \n\t"
#endif
          " and %1, %0;               \n\t"
          : "=r" (mask), "+r" (key)
          : "m" (m_simd[0]), "m" (m_simd[1]), "m" (m_mask)
          : "xmm1", "xmm2", "cc"
          );
          return mask;
        }
      };

      template <> struct int32_keys<16> : public int32_key_holder<16>
      {
        inline uint32_t keymask(int32_t key)
        {
          uint32_t mask;
          __asm__ __volatile__ (
#ifdef __AVX__
          " vmovd   %1, %%xmm4;       \n\t"
          " vshufps $0, %%xmm4, %%xmm4, %%xmm4 \n\t"
          " vpcmpeqd %2, %%xmm4, %%xmm1; \n\t"
          " vpcmpeqd %3, %%xmm4, %%xmm2; \n\t"
          " vpcmpeqd %4, %%xmm4, %%xmm3; \n\t"
          " mov %6, %0;               \n\t"
          " vpackssdw %%xmm2, %%xmm1, %%xmm1;  \n\t"
          " vpcmpeqd %5, %%xmm4, %%xmm4; \n\t"
          " not %0;                   \n\t"
          " vpackssdw %%xmm4, %%xmm3, %%xmm3;  \n\t"
          " vpacksswb %%xmm3, %%xmm1, %%xmm1;  \n\t"
          " vpmovmskb %%xmm1, %1;     \n\t"
#else
          " movdqa %2, %%xmm1;        \n\t"
          " movdqa %3, %%xmm2;        \n\t"
          " movd   %1, %%xmm4;        \n\t"
          " shufps $0, %%xmm4, %%xmm4 \n\t"
          " pcmpeqd %%xmm4, %%xmm1;   \n\t"
          " movdqa %%xmm4, %%xmm3;    \n\t"
          " pcmpeqd %%xmm4, %%xmm2;   \n\t"
          " pcmpeqd %4, %%xmm3;       \n\t"
          " mov %6, %0;               \n\t"
          " packssdw %%xmm2, %%xmm1;  \n\t"
          " pcmpeqd %5, %%xmm4;       \n\t"
          " not %0;                   \n\t"
          " packssdw %%xmm4, %%xmm3;  \n\t"
          " packsswb %%xmm3, %%xmm1;  \n\t"
          " pmovmskb %%xmm1, %1;      \n\t"
#endif
          " and %1, %0;               \n\t"
          : "=r" (mask), "+r" (key)
          : "m" (m_simd[0]), "m" (m_simd[1]), "m" (m_simd[2]), "m" (m_simd[3]), "m" (m_mask)
          : "xmm1", "xmm2", "xmm3", "xmm4", "cc"
          );
          return mask;
        }
      };

      template <typename F, std::size_t N> class IntPtrCache : private int32_keys<N>
      {
        typedef int32_keys<N> Base;
        F* m_p[N + 1];
  
        public:

	static inline std::size_t capacity()
        { return N; }

	void clear()
        { this->m_mask = ((uint32_t)1 << N) - 1; }
  
        F* lookup(int32_t key)
        { return this->lookup_pos(Base::keymask(key), m_p); }

        std::size_t insert(int32_t key, F* p)
        { return this->insert_key(key, p, m_p); }

        F* erase(int32_t key) 
        { return this->erase_pos(Base::keymask(key), m_p); }

        F* evict(std::size_t pos)
        { return this->evict_pos((uint32_t)pos, m_p); }
      };
    }

    template <typename F> class PtrCache<int32_t, F, 16> : public detail::IntPtrCache<F, 16> {};
    template <typename F> class PtrCache<int32_t, F, 8> : public detail::IntPtrCache<F, 8> {};

#endif

  } // namespace FIX::Util

  struct Sg
  {

#ifdef _MSC_VER
    typedef WSABUF   sg_buf_t;
    typedef LPWSABUF sg_buf_ptr;
    typedef CHAR*    sg_ptr_t;

#define IOV_BUF_INITIALIZER(ptr, len) { (ULONG)(len), (CHAR*)(ptr) } 
#define IOV_BUF(b) ((b).buf)
#define IOV_LEN(b) ((b).len)

    static inline void NOTHROW append(sg_buf_t& buf, const char* s, std::size_t len)
    {
      char* pc = (char*)IOV_BUF(buf) + IOV_LEN(buf);
      IOV_LEN(buf) += len;
      while(len--) *pc++ = *s++;
    }

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

#define IOV_BUF_INITIALIZER(ptr, len) { (void*)(ptr), (size_t)(len) } 
#define IOV_BUF(b) ((b).iov_base)
#define IOV_LEN(b) ((b).iov_len)

    // optimize for short strings
    static inline void NOTHROW append(sg_buf_t& buf, const char* s, std::size_t len)
    {
      char* p = (char*)IOV_BUF(buf) + IOV_LEN(buf);
      IOV_LEN(buf) += len;
#if defined(_MSC_VER) || (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)))
      if (len == 1) {
	p[0] = s[0];
      } else if (LIKELY(len > 0)) {
	if (len <= 4) {
		*(uint16_t*)p= *(uint16_t*)s; 
		*(uint16_t*)(p + len - 2) = *(uint16_t*)(s + len - 2); 
	} else if (len <= 8) {
		*(uint32_t*)p= *(uint32_t*)s;
		*(uint32_t*)(p + len - 4) = *(uint32_t*)(s + len - 4); 
	} else if (len <= 16) {
		*(uint64_t*)p= *(uint64_t*)s;
		*(uint64_t*)(p + len - 8) = *(uint64_t*)(s + len - 8);
	} else if (LIKELY(len <= 24)) {
		_mm_storeu_si128((__m128i*)p, _mm_loadu_si128((__m128i*)s));
		*(uint64_t*)(p + len - 8) = *(uint64_t*)(s + len - 8);
	} else {
		std::size_t tmp = ((len + 15) >> 4) - 1;
		for (std::size_t i = 0; i < tmp; i++) {
		  _mm_storeu_si128((__m128i*)p + i, _mm_loadu_si128((__m128i*)s + i));
		}
		_mm_storeu_si128((__m128i*)(p + len - 16), _mm_loadu_si128((__m128i*)(s + len - 16)));
	}
      }
#else
      while(len--) *p++ = *s++;
#endif
    }

    static inline std::size_t NOTHROW send(int socket, sg_buf_ptr bufs, int n)
    {
      struct msghdr m = { NULL, 0, bufs, (std::size_t)n, NULL, 0, 0 };
      ssize_t sz = ::sendmsg(socket, &m, 0);
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
    static std::size_t NOTHROW size( const B& buf )
    {
      return IOV_LEN(buf);
    }

    template <typename P, typename B>
    static P NOTHROW data( B buf )
    {
      return (P)IOV_BUF(buf);
    }

    template <typename B>
    static B split( B& buf, B by )
    {
      char* pe = (char*)IOV_BUF(by) + IOV_LEN(by);
      IOV_BUF(by) = IOV_BUF(buf);
      IOV_LEN(buf) -= pe - (char*)IOV_BUF(buf);
      IOV_BUF(buf) = pe;
      IOV_LEN(by) = pe - (char*)IOV_BUF(by);
      return by;
    }

    template <typename B>
    static std::string toString( const B bufs, int n )
    {
      std::string s;
      for (int i = 0; i < n; i++)
       	s.append((const char*)IOV_BUF(bufs[i]), IOV_LEN(bufs[i]));
      return s;
    }

    template <typename B>
    static std::string toString( B buf )
    {
      return std::string( (const char*)IOV_BUF(buf), IOV_LEN(buf) );
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

#ifdef _MSC_VER
#pragma warning( default : 4706 ) // RE-ENABLE warning C4706: assignment within conditional expression
#endif

#endif
