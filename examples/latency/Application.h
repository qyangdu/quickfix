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

#ifndef LATENCY_APPLICATION_H
#define LATENCY_APPLICATION_H

#include <time.h>
#include <semaphore.h>
#include <tbb/atomic.h>

#include <queue>
#include <iostream>

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Utility.h"
#include "quickfix/Mutex.h"

#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix42/OrderCancelRequest.h"
#include "quickfix/fix42/MarketDataRequest.h"
#include "quickfix/fix43/MarketDataRequest.h"

class Application
      : public FIX::Application
{
  struct timespec  tick_;
  size_t	   num_spans_;
  uint64_t*	   spans_;
  uint64_t	   totalNsec_;
  sem_t		   ready_;
  tbb::atomic<int> logonCount_;
  tbb::atomic<int> receiveCount_;

public:
  Application(uint64_t NumSpans);
  virtual ~Application() { sem_destroy(&ready_); delete [] spans_; }

  // Application overloads
  void onCreate( const FIX::SessionID& ) {}
  void onLogon( const FIX::SessionID& sessionID );
  void onLogout( const FIX::SessionID& sessionID );
  void toAdmin( FIX::Message&, const FIX::SessionID& ) {}
  void toApp( FIX::Message&, const FIX::SessionID& )
  throw( FIX::DoNotSend ) {}
  void fromAdmin( const FIX::Message&, const FIX::SessionID& )
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon ) {}
  void fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
  throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType );

  bool initialized() const { return logonCount_ == 2; }
  void acquire() {
    sem_wait(&ready_);
    clock_gettime(CLOCK_REALTIME, &tick_);
  }
  void release(int seq) {
    struct timespec tock;
    clock_gettime(CLOCK_REALTIME, &tock); 
    uint64_t span = (tock.tv_sec - tick_.tv_sec) * 1000000000 + (tock.tv_nsec - tick_.tv_nsec);
    spans_[seq] = span;
    totalNsec_ += span;
    sem_post(&ready_);
  } 
  int  received() const { return receiveCount_; }
  uint64_t elapsed() const { return totalNsec_; }

  uint64_t* spans() {
    std::sort(spans_, spans_ + num_spans_);
    return spans_;
  }
};

#endif
