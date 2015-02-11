#include "messagefacility/MessageLogger/ErrorObj.h"
#include "messagefacility/MessageLogger/MessageDrop.h"
#include "messagefacility/MessageService/ELdestination.h"

#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/sqlite_DBmanager.h"

#include <cstdint>
#include <memory>

namespace sqlite {

  using std::string;
  using std::uint32_t;
  using ntuple::Ntuple;

  //==========================================================
  // sqlite plugin declaration
  //==========================================================

  class sqlite3Plugin : public mf::service::ELdestination {
  public:

    sqlite3Plugin( const fhicl::ParameterSet& pset );

  private:

    virtual void fillPrefix  (       std::ostringstream&, const mf::ErrorObj& ) override {}
    virtual void fillSuffix  (       std::ostringstream&, const mf::ErrorObj& ) override {}
    virtual void routePayload( const std::ostringstream&, const mf::ErrorObj& ) override;

    DBmanager dbMgr_;
    Ntuple<string,string,string,string,string,string,unsigned,string,string> msgHeadersTable_;
    Ntuple<sqlite_int64,string> usrMessagesTable_;

  };

  // Implementation
  //===============================================================================================================
  sqlite3Plugin::sqlite3Plugin(const fhicl::ParameterSet& pset)
    : ELdestination( pset )
    , dbMgr_ ( pset.get<std::string>("filename") )
    , msgHeadersTable_ (dbMgr_.get(), "MessageHeaders",
                        {"Timestamp","Hostname","Hostaddress","Severity","Category","AppName","ProcessId","RunEventNo","ModuleName"} )
    , usrMessagesTable_(dbMgr_.get(), "UserMessages",
                        {"HeaderId","Message"} )
  {}


  //===============================================================================================================
  void sqlite3Plugin::routePayload( const std::ostringstream& oss, const mf::ErrorObj& msg ) {

    const auto& xid = msg.xid();

    const string& timestamp  = format.timestamp( msg.timestamp() );    // timestamp
    const string& hostname   = xid.hostname;                           // host name
    const string& hostaddr   = xid.hostaddr;                           // host address
    const string& severity   = xid.severity.getName();                 // severity
    const string& category   = xid.id;                                 // category
    const string& app        = xid.application;                        // application
    const long&   pid        = xid.pid;                                // process id
    const string& runEventNo = mf::MessageDrop::instance()->runEvent;  // run/event no
    const string& modname    = xid.module;                             // module name
    const string& usrMsg     =
      !oss.str().compare(0,1,"\n") ? oss.str().erase(0,1) : oss.str(); // user-supplied msg
                                                                       // (remove leading "\n" if present)
    msgHeadersTable_.insert( timestamp,
                             hostname,
                             hostaddr,
                             severity,
                             category,
                             app,
                             pid,
                             runEventNo,
                             modname );

    usrMessagesTable_.insert(msgHeadersTable_.lastRowid(),usrMsg);

  }

}

//======================================================================
//
// makePlugin function
//
//======================================================================

extern "C" {

  auto makePlugin( const std::string&,
                   const fhicl::ParameterSet& pset) {

    return std::make_unique<sqlite::sqlite3Plugin>( pset );

  }

}
DEFINE_BASIC_PLUGINTYPE_FUNC(mf::service::ELdestination)


