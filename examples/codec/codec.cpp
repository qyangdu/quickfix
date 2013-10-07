#include "quickfix/Field.h"
#include "quickfix/Values.h"
#include "quickfix/Message.h"


//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    FIX::Message msg;
    msg.getHeader().setField( FIX::BeginString( "FIX.4.4" ) );
    msg.getHeader().setField( FIX::MsgType( FIX::MsgType_NewOrderSingle ) );
    msg.getHeader().setField( FIX::MsgSeqNum(78) );
    msg.getHeader().setField( FIX::SenderCompID("A12345B") );
    msg.getHeader().setField( FIX::SenderSubID("2DEFGH4") );
    msg.getHeader().setField( FIX::SendingTime(FIX::UtcTimeStamp()) );
    msg.getHeader().setField( FIX::TargetCompID("COMPARO") );
    msg.getHeader().setField( FIX::TargetSubID("G") );
    msg.getHeader().setField( FIX::SenderLocationID("AU,SY") );

    msg.setField( FIX::Account( "01234567") );
    msg.setField( FIX::ClOrdID( "4" ) );
    msg.setField( FIX::OrderQty( 50 ) );
    msg.setField( FIX::OrdType( FIX::OrdType_LIMIT) );
    msg.setField( FIX::Price( 400.5) );
    msg.setField( FIX::HandlInst( '1' ) );
    msg.setField( FIX::Symbol( "OC") );
    msg.setField( FIX::Text( "NIGEL") );
    msg.setField( FIX::Side( FIX::Side_BUY ) );
    msg.setField( FIX::SecurityDesc( "AOZ3 C02000") );
    msg.setField( FIX::TimeInForce( FIX::TimeInForce_DAY ) );
    msg.setField( FIX::TransactTime() );
    msg.setField( FIX::SecurityType( FIX::SecurityType_OPTION ) );

    std::string output;
    double tt = 0;
    for(int i = 0 ; i < 1000000; ++i)
    {
        FIX::Message msg(true);
        msg.getHeader().setField( FIX::BeginString::Pack( "FIX.4.4" ) );
        msg.getHeader().setField( FIX::MsgType::Pack( FIX::MsgType_NewOrderSingle ) );
        msg.getHeader().setField( FIX::MsgSeqNum::Pack(78));
        msg.getHeader().setField( FIX::SenderCompID::Pack("A12345B"));
        msg.getHeader().setField( FIX::SenderSubID::Pack("2DEFGH4"));
        msg.getHeader().setField( FIX::SendingTime::Pack(FIX::UtcTimeStamp()));
        msg.getHeader().setField( FIX::TargetCompID::Pack("COMPARO"));
        msg.getHeader().setField( FIX::TargetSubID::Pack("G"));
        msg.getHeader().setField( FIX::SenderLocationID::Pack("AU,SY"));

        msg.setField( FIX::Account::Pack( "01234567") );
        msg.setField( FIX::ClOrdID::Pack( "4" ) );
        msg.setField( FIX::OrderQty::Pack( 50 ) );
        msg.setField( FIX::OrdType::Pack( FIX::OrdType_LIMIT) );
        msg.setField( FIX::Price::Pack( 400.5) );
        msg.setField( FIX::HandlInst::Pack( '1' ) );
        msg.setField( FIX::Symbol::Pack( "OC") );
        msg.setField( FIX::Text::Pack( "NIGEL") );
        msg.setField( FIX::Side::Pack( FIX::Side_BUY ) );
        msg.setField( FIX::SecurityDesc::Pack( "AOZ3 C02000") );
        msg.setField( FIX::TimeInForce::Pack( FIX::TimeInForce_DAY ) );
        msg.setField( FIX::TransactTime() );
        msg.setField( FIX::SecurityType::Pack( FIX::SecurityType_OPTION ) );

        FIX::Util::Sys::TickCount start = FIX::Util::Sys::TickCount::now();
        msg.toString(output);
        FIX::Util::Sys::TickCount end = FIX::Util::Sys::TickCount::now();

        tt += (end-start).seconds();
    }

    std::cout << "to string - " << tt << std::endl;

    if( argc > 1 )
    {
      if( FIX::file_exists(argv[1]) )
      {
        FIX::DataDictionary dict(argv[1]);

        FIX::Util::Sys::TickCount start = FIX::Util::Sys::TickCount::now();
        for(int i = 0 ; i < 1000000; ++i)
        {
          FIX::Message * tmp = new FIX::Message(output, dict);
          delete tmp;
        }
        FIX::Util::Sys::TickCount end = FIX::Util::Sys::TickCount::now();

        std::cout << "from string - " << (end-start).seconds() << std::endl;
      }
      else
        std::cout << "Unable to open dictionary file " << argv[1] << std::endl;
    }
  return 0;
}

