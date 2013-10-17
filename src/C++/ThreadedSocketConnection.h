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

#ifndef FIX_THREADEDSOCKETCONNECTION_H
#define FIX_THREADEDSOCKETCONNECTION_H

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#endif

#include "Parser.h"
#include "Responder.h"
#include "SessionID.h"
#include <set>
#include <map>

namespace FIX
{
class ThreadedSocketAcceptor;
class ThreadedSocketInitiator;
class Session;
class Application;
class Log;

/// Encapsulates a socket file descriptor (multi-threaded).
class ThreadedSocketConnection : Responder
{
public:
  typedef std::set<SessionID> Sessions;

  ThreadedSocketConnection( int s, Sessions sessions, Application& application, Log* pLog );
  ThreadedSocketConnection( const SessionID&, int s, 
                            const std::string& address, short port, 
                            Application&, Log* pLog );
  virtual ~ThreadedSocketConnection() ;

  Session* getSession() const { return m_pSession; }
  int getSocket() const { return m_socket; }
  bool connect();
  void disconnect();
  bool read();

private:
  bool readMessage( Sg::sg_buf_t& msg );
  void processStream();
  bool setSession( Sg::sg_buf_t& msg );

  bool send( const std::string& );
  bool send( Sg::sg_buf_ptr bufs, int n );

  int m_poll;
  int m_socket;
  char m_buffer[4 * BUFSIZ];

  UtcTimeStamp m_ts;
  std::string m_address;
  std::string m_msg;
  int m_port;

  Application& m_application;
  Log* m_pLog;
  Parser m_parser;
  Sessions m_sessions;
  Session* m_pSession;
  bool m_disconnect;
  fd_set m_fds;
};
}

#endif //FIX_THREADEDSOCKETCONNECTION_H
