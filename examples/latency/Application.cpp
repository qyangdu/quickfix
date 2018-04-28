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

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#else
#include "config.h"
#endif

#include "Application.h"
#include "quickfix/Session.h"

#include "quickfix/fix42/ExecutionReport.h"

Application::Application(std::size_t N) 
: tick_(FIX::Util::Sys::TickCount::now()), ready_(1),
  logonCount_(0), receiveCount_(0)
{
  totalSec_ = 0;
  spans_ = new double[N];
  num_spans_ = N;
}

void Application::onLogon( const FIX::SessionID& sessionID ) { ++logonCount_; }

void Application::onLogout( const FIX::SessionID& sessionID ) { ++logonCount_; }

void Application::fromApp( const FIX::Message& message,
                           const FIX::SessionID& sessionID )
throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
  release(++receiveCount_ - 1);
}

