#include "messagefacility/Utilities/ErrorObj.h"
#include "messagefacility/MessageLogger/MessageDrop.h"
#include "messagefacility/MessageService/ELdestination.h"

#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/sqlite_DBmanager.h"

#include <cstdint>
#include <memory>

namespace mf {
  namespace service {
    class ELcontextSupplier;
  }
}

namespace sqlite {

  using std::string;
  using std::uint32_t;
  using ntuple::Ntuple;

  //==========================================================
  // sqlite plugin declaration
  //==========================================================

  class sqlite3Plugin : public mf::service::ELdestination {
  public:

    sqlite3Plugin(fhicl::ParameterSet const&);

  private:

    virtual void fillPrefix(std::ostringstream&,
                            mf::ErrorObj const&,
                            mf::service::ELcontextSupplier const&) override
    {}

    virtual void fillSuffix(std::ostringstream&,
                            mf::ErrorObj const&) override
    {}

    virtual void routePayload(std::ostringstream const&,
                              mf::ErrorObj const&,
                              mf::service::ELcontextSupplier const&) override;

    DBmanager dbMgr_;
    Ntuple<string,string,string,string,string,string,unsigned,string,string> msgHeadersTable_;
    Ntuple<sqlite_int64,string> usrMessagesTable_;
  };

  // Implementation
  //===============================================================================================================
  sqlite3Plugin::sqlite3Plugin(fhicl::ParameterSet const& pset)
    : ELdestination{pset}
    , dbMgr_{pset.get<std::string>("filename")}
    , msgHeadersTable_{dbMgr_, "MessageHeaders", {"Timestamp","Hostname","Hostaddress","Severity","Category","AppName","ProcessId","RunEventNo","ModuleName"}}
    , usrMessagesTable_{dbMgr_, "UserMessages", {"HeaderId","Message"}}
  {}


  //===============================================================================================================
  void sqlite3Plugin::routePayload(std::ostringstream const& oss,
                                   mf::ErrorObj const& msg,
                                   mf::service::ELcontextSupplier const&)
  {
    auto const& xid = msg.xid();

    string const& timestamp  = format.timestamp( msg.timestamp() );    // timestamp
    string const& hostname   = xid.hostname;                           // host name
    string const& hostaddr   = xid.hostaddr;                           // host address
    string const& severity   = xid.severity.getName();                 // severity
    string const& category   = xid.id;                                 // category
    string const& app        = xid.application;                        // application
    long   const& pid        = xid.pid;                                // process id
    string const& runEventNo = mf::MessageDrop::instance()->runEvent;  // run/event no
    string const& modname    = xid.module;                             // module name
    string const& usrMsg     =
      !oss.str().compare(0,1,"\n") ? oss.str().erase(0,1) : oss.str(); // user-supplied msg
                                                                       // (remove leading "\n" if present)
    msgHeadersTable_.insert(timestamp,
                            hostname,
                            hostaddr,
                            severity,
                            category,
                            app,
                            pid,
                            runEventNo,
                            modname);

    usrMessagesTable_.insert(msgHeadersTable_.lastRowid(), usrMsg);
  }

}

//======================================================================
//
// makePlugin function
//
//======================================================================

extern "C" {

  auto makePlugin(std::string const&, fhicl::ParameterSet const& pset)
  {
    return std::make_unique<sqlite::sqlite3Plugin>( pset );
  }

}
DEFINE_BASIC_PLUGINTYPE_FUNC(mf::service::ELdestination)
