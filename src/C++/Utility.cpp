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

#ifdef _MSC_VER
#include "stdafx.h"
#else
#include "config.h"
#endif

#include "Utility.h"

#ifdef USING_STREAMS
#include <stropts.h>
#include <sys/conf.h>
#endif
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>

namespace FIX
{

ALIGN_DECL_DEFAULT const int detail::bitop_base::Mod67Position[] = {
  64, 0, 1, 39, 2, 15, 40, 23, 3, 12, 16, 59, 41, 19, 24, 54,
  4, 64, 13, 10, 17, 62, 60, 28, 42, 30, 20, 51, 25, 44, 55,
  47, 5, 32, 65, 38, 14, 22, 11, 58, 18, 53, 63, 9, 61, 27,
  29, 50, 43, 46, 31, 37, 21, 57, 52, 8, 26, 49, 45, 36, 56,
  7, 48, 35, 6, 34, 33, 0
};

#if (defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))) || defined(_MSC_VER)
// cacheline aligned block of magic constants
ALIGN_DECL(64) Util::x86Data::ConvBits Util::x86Data::cbits =
{
  { // mul_10
   10, 10, 10, 10, 10, 10, 10, 10
  },
  { // div_const
    8389, 5243, 13108, 0x8000, 8389, 5243, 13108, 0x8000
  },
  { // shl_const
    1 << (16 - (23 + 2 - 16)),
    1 << (16 - (19 + 2 - 16)),
    1 << (16 - 1 - 2),
    1 << (15),
    1 << (16 - (23 + 2 - 16)),
    1 << (16 - (19 + 2 - 16)),
    1 << (16 - 1 - 2),
    1 << (15)
  },
  { // to_ascii 
    '0', '0', '0', '0', '0', '0', '0', '0', 
    '0', '0', '0', '0', '0', '0', '0', '0'
  }
};

ALIGN_DECL_DEFAULT Util::x86Data::Log2 Util::x86Data::m_digits[32] = 
{
  { { 1, 9 } },
  { { 1, 9 } },
  { { 1, 9 } },
  { { 1, 9 } },
  { { 2, 99 } },
  { { 2, 99 } },
  { { 2, 99 } },
  { { 3, 999 } },
  { { 3, 999 } },
  { { 3, 999 } },
  { { 4, 9999 } },
  { { 4, 9999 } },
  { { 4, 9999 } },
  { { 4, 9999 } },
  { { 5, 99999 } },
  { { 5, 99999 } },
  { { 5, 99999 } },
  { { 6, 999999 } },
  { { 6, 999999 } },
  { { 6, 999999 } },
  { { 7, 9999999 } },
  { { 7, 9999999 } },
  { { 7, 9999999 } },
  { { 7, 9999999 } },
  { { 8, 99999999 } },
  { { 8, 99999999 } },
  { { 8, 99999999 } },
  { { 9, 999999999 } },
  { { 9, 999999999 } },
  { { 9, 999999999 } },
  { { 10, (std::numeric_limits<uint32_t>::max)() } },
  { { 10, (std::numeric_limits<uint32_t>::max)() } }
};

ALIGN_DECL_DEFAULT Util::NumData::DigitPair Util::NumData::m_pairs[100] =
{
{ {'0','0'} }, { {'0','1'} }, { {'0','2'} }, { {'0','3'} }, { {'0','4'} }, { {'0','5'} }, { {'0','6'} }, { {'0','7'} }, { {'0','8'} }, { {'0','9'} },
{ {'1','0'} }, { {'1','1'} }, { {'1','2'} }, { {'1','3'} }, { {'1','4'} }, { {'1','5'} }, { {'1','6'} }, { {'1','7'} }, { {'1','8'} }, { {'1','9'} },
{ {'2','0'} }, { {'2','1'} }, { {'2','2'} }, { {'2','3'} }, { {'2','4'} }, { {'2','5'} }, { {'2','6'} }, { {'2','7'} }, { {'2','8'} }, { {'2','9'} },
{ {'3','0'} }, { {'3','1'} }, { {'3','2'} }, { {'3','3'} }, { {'3','4'} }, { {'3','5'} }, { {'3','6'} }, { {'3','7'} }, { {'3','8'} }, { {'3','9'} },
{ {'4','0'} }, { {'4','1'} }, { {'4','2'} }, { {'4','3'} }, { {'4','4'} }, { {'4','5'} }, { {'4','6'} }, { {'4','7'} }, { {'4','8'} }, { {'4','9'} },
{ {'5','0'} }, { {'5','1'} }, { {'5','2'} }, { {'5','3'} }, { {'5','4'} }, { {'5','5'} }, { {'5','6'} }, { {'5','7'} }, { {'5','8'} }, { {'5','9'} },
{ {'6','0'} }, { {'6','1'} }, { {'6','2'} }, { {'6','3'} }, { {'6','4'} }, { {'6','5'} }, { {'6','6'} }, { {'6','7'} }, { {'6','8'} }, { {'6','9'} },
{ {'7','0'} }, { {'7','1'} }, { {'7','2'} }, { {'7','3'} }, { {'7','4'} }, { {'7','5'} }, { {'7','6'} }, { {'7','7'} }, { {'7','8'} }, { {'7','9'} },
{ {'8','0'} }, { {'8','1'} }, { {'8','2'} }, { {'8','3'} }, { {'8','4'} }, { {'8','5'} }, { {'8','6'} }, { {'8','7'} }, { {'8','8'} }, { {'8','9'} },
{ {'9','0'} }, { {'9','1'} }, { {'9','2'} }, { {'9','3'} }, { {'9','4'} }, { {'9','5'} }, { {'9','6'} }, { {'9','7'} }, { {'9','8'} }, { {'9','9'} }
};

ALIGN_DECL_DEFAULT Util::ULong::Log2 Util::ULong::m_digits[64] =
{
  { 9, 1 },
  { 9, 1 },
  { 9, 1 },
  { 9, 1 },
  { 99, 2 },
  { 99, 2 },
  { 99, 2 },
  { 999, 3 },
  { 999, 3 },
  { 999, 3 },
  { 9999, 4 },
  { 9999, 4 },
  { 9999, 4 },
  { 9999, 4 },
  { 99999, 5 },
  { 99999, 5 },
  { 99999, 5 },
  { 999999, 6 },
  { 999999, 6 },
  { 999999, 6 },
  { 9999999, 7 },
  { 9999999, 7 },
  { 9999999, 7 },
  { 9999999, 7 },
  { 99999999, 8 },
  { 99999999, 8 },
  { 99999999, 8 },
  { 999999999, 9 },
  { 999999999, 9 },
  { 999999999, 9 },
  { 9999999999LL, 10 },
  { 9999999999LL, 10 },
  { 9999999999LL, 10 },
  { 9999999999LL, 10 },
  { 99999999999LL, 11 },
  { 99999999999LL, 11 },
  { 99999999999LL, 11 },
  { 999999999999LL, 12 },
  { 999999999999LL, 12 },
  { 999999999999LL, 12 },
  { 9999999999999LL, 13 },
  { 9999999999999LL, 13 },
  { 9999999999999LL, 13 },
  { 9999999999999LL, 13 },
  { 99999999999999LL, 14 },
  { 99999999999999LL, 14 },
  { 99999999999999LL, 14 },
  { 999999999999999LL, 15 },
  { 999999999999999LL, 15 },
  { 999999999999999LL, 15 },
  { 9999999999999999LL, 16 },
  { 9999999999999999LL, 16 },
  { 9999999999999999LL, 16 },
  { 9999999999999999LL, 16 },
  { 99999999999999999LL, 17 },
  { 99999999999999999LL, 17 },
  { 99999999999999999LL, 17 },
  { 999999999999999999LL, 18 },
  { 999999999999999999LL, 18 },
  { 999999999999999999LL, 18 },
  { 9999999999999999999ULL, 19 },
  { 9999999999999999999ULL, 19 },
  { 9999999999999999999ULL, 19 },
  { 9999999999999999999ULL, 19 }
};
#endif

void string_replace( const std::string& oldValue,
                     const std::string& newValue,
                     std::string& value )
{
  for( std::string::size_type pos = value.find(oldValue);
       pos != std::string::npos;
       pos = value.find(oldValue, pos) )
  {
    value.replace( pos, oldValue.size(), newValue );
    pos += newValue.size();
  }
}

std::string string_toUpper( const std::string& value )
{
  std::string copy = value;
  std::transform( copy.begin(), copy.end(), copy.begin(), ::toupper );
  return copy;
}

std::string string_toLower( const std::string& value )
{
  std::string copy = value;
  std::transform( copy.begin(), copy.end(), copy.begin(), ::tolower );
  return copy;
}

std::string string_strip( const std::string& value )
{
  if( !value.size() )
    return value;

  size_t startPos = value.find_first_not_of(" \t\r\n");
  size_t endPos = value.find_last_not_of(" \t\r\n");

  if( startPos == std::string::npos )
   return value;

  return std::string( value, startPos, endPos - startPos + 1 );
}

void socket_init()
{
#ifdef _MSC_VER
  WORD version = MAKEWORD( 2, 2 );
  WSADATA data;
  WSAStartup( version, &data );
#else
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = 0;
  sigaction( SIGPIPE, &sa, 0 );
#endif
}

void socket_term()
{
#ifdef _MSC_VER
  WSACleanup();
#endif
}

int socket_createAcceptor(int port, bool reuse)
{
  int socket = ::socket( PF_INET, SOCK_STREAM, 0 );
  if ( socket < 0 ) return -1;

  sockaddr_in address;
  socklen_t socklen;

  address.sin_family = PF_INET;
  address.sin_port = htons( port );
  address.sin_addr.s_addr = INADDR_ANY;
  socklen = sizeof( address );
  if( reuse )
    socket_setsockopt( socket, SO_REUSEADDR );

  int result = bind( socket, reinterpret_cast < sockaddr* > ( &address ),
                     socklen );
  if ( result < 0 ) return -1;
#ifdef TCP_QUICKACK
  int optval = 1;
  ::setsockopt( socket, IPPROTO_TCP, TCP_QUICKACK,
                       &optval, sizeof( optval ) );
#endif
  result = listen( socket, SOMAXCONN );
  if ( result < 0 ) return -1;
  return socket;
}

int socket_createConnector()
{
  return ::socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
}

int socket_connect( int socket, const char* address, int port )
{
  const char* hostname = socket_hostname( address );
  if( hostname == 0 ) return -1;

  sockaddr_in addr;
  addr.sin_family = PF_INET;
  addr.sin_port = htons( port );
  addr.sin_addr.s_addr = inet_addr( hostname );

  int result = connect( socket, reinterpret_cast < sockaddr* > ( &addr ),
                        sizeof( addr ) );

  return result;
}

int socket_accept( int s )
{
  if ( !socket_isValid( s ) ) return -1;
  return accept( s, 0, 0 );
}

ssize_t socket_send( int s, const char* msg, size_t length )
{
  return send( s, msg, length, 0 );
}

void socket_close( int s )
{
  shutdown( s, 2 );
#ifdef _MSC_VER
  closesocket( s );
#else
  close( s );
#endif
}

bool socket_fionread( int s, int& bytes )
{
  bytes = 0;
#if defined(_MSC_VER)
  return ::ioctlsocket( s, FIONREAD, &( ( unsigned long& ) bytes ) ) == 0;
#elif defined(USING_STREAMS)
  return ::ioctl( s, I_NREAD, &bytes ) >= 0;
#else
  return ::ioctl( s, FIONREAD, &bytes ) == 0;
#endif
}

bool socket_disconnected( int s )
{
  char byte;
  return ::recv (s, &byte, sizeof (byte), MSG_PEEK) <= 0;
}

int socket_setsockopt( int s, int opt )
{
#ifdef _MSC_VER
  BOOL optval = TRUE;
#else
  int optval = 1;
#endif
  return socket_setsockopt( s, opt, optval );
}

int socket_setsockopt( int s, int opt, int optval )
{
  int level = SOL_SOCKET;
  if( opt == TCP_NODELAY )
    level = IPPROTO_TCP;

#ifdef _MSC_VER
  if( opt == TCP_NODELAY )
  {
	  DWORD numBytes;
#ifdef SIO_LOOPBACK_FAST_PATH
	  int siopt = SIO_LOOPBACK_FAST_PATH;
#else
	  int siopt = (-1744830448);
#endif
	  WSAIoctl(s, siopt, &optval, sizeof(optval), NULL, 0, &numBytes, 0, 0);
  }
  return ::setsockopt( s, level, opt,
                       ( char* ) & optval, sizeof( optval ) );
#else
#ifdef IPTOS_LOWDELAY
  if (opt == TCP_NODELAY)
  {
    int tos = IPTOS_LOWDELAY;
    ::setsockopt( s, IPPROTO_IP, IP_TOS,
                       &tos, sizeof(tos) );
  }
#endif
#ifdef SO_PRIORITY
  if (opt == TCP_NODELAY)
  {
    int prio = 6;
    ::setsockopt( s, SOL_SOCKET, SO_PRIORITY,
                       &prio, sizeof(prio) );
  }
#endif
  return ::setsockopt( s, level, opt,
                       &optval, sizeof( optval ) );
#endif
}

int socket_getsockopt( int s, int opt, int& optval )
{
  int level = SOL_SOCKET;
  if( opt == TCP_NODELAY )
    level = IPPROTO_TCP;

#ifdef _MSC_VER
  int length = sizeof(int);
#else
  socklen_t length = sizeof(socklen_t);
#endif

  return ::getsockopt( s, level, opt, 
                       ( char* ) & optval, & length );
}

#ifndef _MSC_VER
int socket_fcntl( int s, int opt, int arg )
{
  return ::fcntl( s, opt, arg );
}

int socket_getfcntlflag( int s, int arg )
{
  return socket_fcntl( s, F_GETFL, arg );
}

int socket_setfcntlflag( int s, int arg )
{
  int oldValue = socket_getfcntlflag( s, arg );
  oldValue |= arg;
  return socket_fcntl( s, F_SETFL, arg );
}
#endif

void socket_setnonblock( int socket )
{
#ifdef _MSC_VER
  u_long opt = 1;
  ::ioctlsocket( socket, FIONBIO, &opt );
#else
  socket_setfcntlflag( socket, O_NONBLOCK );
#endif
}

bool socket_isValid( int socket )
{
#ifdef _MSC_VER
  return socket != INVALID_SOCKET;
#else
  return socket >= 0;
#endif
}

#ifndef _MSC_VER
bool socket_isBad( int s )
{
  struct stat buf;
  fstat( s, &buf );
  return errno == EBADF;
}
#endif

void socket_invalidate( int& socket )
{
#ifdef _MSC_VER
  socket = INVALID_SOCKET;
#else
  socket = -1;
#endif
}

short socket_hostport( int socket )
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if( getsockname(socket, (struct sockaddr*)&addr, &len) < 0 )
    return 0;

  return ntohs( addr.sin_port );
}

const char* socket_hostname( int socket )
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if( getsockname(socket, (struct sockaddr*)&addr, &len) < 0 )
    return 0;

  return inet_ntoa( addr.sin_addr );
}

const char* socket_hostname( const char* name )
{
  struct hostent* host_ptr = 0;
  struct in_addr** paddr;
  struct in_addr saddr;

#if( GETHOSTBYNAME_R_INPUTS_RESULT || GETHOSTBYNAME_R_RETURNS_RESULT )
  hostent host;
  char buf[1024];
  int error;
#endif

  saddr.s_addr = inet_addr( name );
  if ( saddr.s_addr != ( unsigned ) - 1 ) return name;

#if GETHOSTBYNAME_R_INPUTS_RESULT
  gethostbyname_r( name, &host, buf, sizeof(buf), &host_ptr, &error );
#elif GETHOSTBYNAME_R_RETURNS_RESULT
  host_ptr = gethostbyname_r( name, &host, buf, sizeof(buf), &error );
#else
  host_ptr = gethostbyname( name );
#endif

  if ( host_ptr == 0 ) return 0;

  paddr = ( struct in_addr ** ) host_ptr->h_addr_list;
  return inet_ntoa( **paddr );
}

const char* socket_peername( int socket )
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if( getpeername( socket, (struct sockaddr*)&addr, &len ) < 0 )
    return "UNKNOWN";
  char* result = inet_ntoa( addr.sin_addr );
  if( result )
    return result;
  else
    return "UNKNOWN";
}

std::pair<int, int> socket_createpair()
{
#ifdef _MSC_VER
  int acceptor = socket_createAcceptor(0, true);
  const char* host = socket_hostname( acceptor );
  short port = socket_hostport( acceptor );
  int client = socket_createConnector();
  socket_connect( client, "localhost", port );
  int server = socket_accept( acceptor );
  socket_close(acceptor);
  return std::pair<int, int>( client, server );
#else
  int pair[2];
  socketpair( AF_UNIX, SOCK_STREAM, 0, pair );
  return std::pair<int, int>( pair[0], pair[1] );
#endif
}

tm time_gmtime( const time_t* t )
{
#ifdef _MSC_VER
  #if( _MSC_VER >= 1400 )
    tm result;
    gmtime_s( &result, t );
    return result;
  #else
    return *gmtime( t );
  #endif
#else
  tm result;
  return *gmtime_r( t, &result );
#endif
}

tm time_localtime( const time_t* t)
{
#ifdef _MSC_VER
  #if( _MSC_VER >= 1400 )
    tm result;
    localtime_s( &result, t );
    return result;
  #else
    return *localtime( t );
  #endif
#else
  tm result;
  return *localtime_r( t, &result );
#endif
}

bool thread_spawn( THREAD_START_ROUTINE func, void* var, thread_id& thread )
{
#ifdef _MSC_VER
  thread_id result = 0;
  unsigned int id = 0;
  result = _beginthreadex( NULL, 0, &func, var, 0, &id );
  if ( result == 0 ) return false;
#else
  thread_id result = 0;
  if( pthread_create( &result, 0, func, var ) != 0 ) return false;
#endif
  thread = result;
  return true;
}

bool thread_spawn( THREAD_START_ROUTINE func, void* var )
{ 
  thread_id thread = 0;
  return thread_spawn( func, var, thread );
}

void thread_join( thread_id thread )
{
#ifdef _MSC_VER
  WaitForSingleObject( ( void* ) thread, INFINITE );
  CloseHandle((HANDLE)thread);
#else
  pthread_join( ( pthread_t ) thread, 0 );
#endif
}

void thread_detach( thread_id thread )
{
#ifdef _MSC_VER
  CloseHandle((HANDLE)thread);
#else
  pthread_t t = thread;
  pthread_detach( t );
#endif
}

thread_id thread_self()
{
#ifdef _MSC_VER
  return (unsigned)GetCurrentThread();
#else
  return pthread_self();
#endif
}

void process_sleep( double s )
{
#ifdef _MSC_VER
  Sleep( (long)(s * 1000) );
#else
  timespec time, remainder;
  double intpart;
  time.tv_nsec = (long)(modf(s, &intpart) * 1e9);
  time.tv_sec = (int)intpart;
  while( nanosleep(&time, &remainder) == -1 )
  time = remainder;
#endif
}

std::string file_separator()
{
#ifdef _MSC_VER
  return "\\";
#else
  return "/";
#endif
}

void file_mkdir( const char* path )
{
  int length = (int)strlen( path );
  std::string createPath = "";

  for( const char* pos = path; (pos - path) <= length; ++pos )
  {
    createPath += *pos;
    if( *pos == '/' || *pos == '\\' || (pos - path) == length )
    {
    #ifdef _MSC_VER
      _mkdir( createPath.c_str() );
    #else
      // use umask to override rwx for all
      mkdir( createPath.c_str(), 0777 );
    #endif
    }
  }
}

FILE* file_fopen( const char* path, const char* mode )
{
#if( _MSC_VER >= 1400 )
  FILE* result = 0;
  fopen_s( &result, path, mode );
  return result;
#else
  return fopen( path, mode );
#endif
}

void file_fclose( FILE* file )
{
  fclose( file );
}

bool file_exists( const char* path )
{
  std::ifstream stream;
  stream.open( path, std::ios_base::in );
  if( stream.is_open() )
  {
    stream.close();
    return true;
  }
  return false;
}

void file_unlink( const char* path )
{
#ifdef _MSC_VER
  _unlink( path );
#else
  unlink( path );
#endif

}

int file_rename( const char* oldpath, const char* newpath )
{
  return rename( oldpath, newpath );
}

std::string file_appendpath( const std::string& path, const std::string& file )
{
  const char last = path[path.size()-1];
  if( last == '/' || last == '\\' )
    return std::string(path) + file;
  else
    return std::string(path) + file_separator() + file;
}

FILE_HANDLE_TYPE file_handle_open( const char* path )
{
#ifdef _MSC_VER
    return ::CreateFile( path, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL, NULL);
#else
    return ::open( path, O_CREAT | O_RDWR,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif
}

void file_handle_close( FILE_HANDLE_TYPE handle )
{
#ifdef _MSC_VER
    ::CloseHandle( handle );
#else
    ::close( handle );
#endif
}

long file_handle_read_at( FILE_HANDLE_TYPE handle, char* buf,
                          std::size_t size, FILE_OFFSET_TYPE offset )
{
#ifdef _MSC_VER
    DWORD numRead;
    ::OVERLAPPED overlapped;
    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;
    overlapped.hEvent = 0;
    if( ::ReadFile( handle, buf, size, &numRead, &overlapped ) )
      return (long)numRead;
    return -1;
#else
    return ::pread( handle, buf, size, offset );
#endif
}

long file_handle_write( FILE_HANDLE_TYPE handle, 
                                           const char* buf, std::size_t size )
{
#ifdef _MSC_VER
    DWORD numWritten;
    if( ::WriteFile( handle, buf, size, &numWritten, NULL ) )
      return (long)numWritten;
    return -1;
#else
    return ::write( handle, buf, size );
#endif
}


FILE_OFFSET_TYPE file_handle_seek( FILE_HANDLE_TYPE handle,
                                   FILE_OFFSET_TYPE offset, int whence )
{
#ifdef _MSC_VER
    offset.LowPart = ::SetFilePointer( handle,
                                offset.LowPart, &offset.HighPart, whence );
    if( offset.LowPart == INVALID_SET_FILE_POINTER &&
      ::GetLastError() != NO_ERROR )
      offset.QuadPart = -1;
    return offset;
#else
    return ::lseek( handle, offset, whence );
#endif
}
} // namespace FIX

