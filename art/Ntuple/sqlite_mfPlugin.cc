#include "messagefacility/MessageLogger/ErrorObj.h"
#include "messagefacility/MessageLogger/MessageDrop.h"
#include "messagefacility/MessageService/ELdestination.h"
#include "messagefacility/Utilities/FormatTime.h"

#include "art/Ntuple/Ntuple.h"
#include "art/Ntuple/tuple_helpers.h"

#include <cstdint>
#include <memory>
#include <regex>

//=========================================================================
// sqlite_mfplugin
//
// -----
// Notes
// -----
//
// This plugin makes heavy use of the std::regex library, introduced
// in C++11.  As regex's are used to tokenize a string into a vector
// of substrings, there are many potential uses of (e.g.) \\s, \\d,
// where the "\\" is necessary to suitably escape the special
// character.  To make things more readable, the C++11 raw string
// features have been implemented, so that
//
//   "something\\s+\\."   can be written   R"(something\s+\.)"
//
// In addition, as std::regex_match( str, std::regex( pattern ) )
// requires "pattern" to convertible to an std::string, it is
// preferable to directly send an std::string to the regex matcher.
// For that reason, the C++14 std::string_literals::s string literal
// has been used:
//
//   R"(something\s+\.)"   ==> obj. of type const char* from raw literal
//   R"(something\s+\.)"s  ==> obj. of type std::string from raw literal
//
//=========================================================================

using namespace tupleUtils;
using namespace std::string_literals;

namespace {

  using std::string;
  using std::uint32_t;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Regex utilities

  inline bool check_regex(const string& str, const string& pattern) {
    return std::regex_match( str, std::regex( pattern ) );
  }

  // Regex helper struct
  struct RegexMatchList {
    RegexMatchList(const std::string& str, const std::initializer_list<int>& list ) : regex_pattern(str), match_list(list) {}
    std::string regex_pattern;
    std::initializer_list<int> match_list;
  };

  // Regex patterns for various types
  const RegexMatchList runEventNosRML("(.*)"s+
                                      R"(run:\s(\d+)\s)"s+
                                      R"(subRun:\s(\d+)\s)"s+
                                      R"(event:\s(\d+)\s)"s+
                                      "(.*)"s,
                                      {2,3,4,5}
                                      );

  const RegexMatchList memCheckSplitRML (R"(\s*MemoryCheck:\s(\S*)\s(\S*):(\S*)\s)",{1,2,3,-1} );
  const RegexMatchList memCheckSuffixRML(R"(\s*VSIZE\s(\d+\.?\d*)\s(\d+\.?\d*)\s)"s+
                                         R"(RSS\s(\d+\.?\d*)\s(\d+\.?\d*)(\s*\S*(.*)))"s,
                                         {1,2,3,4,5} );

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Run event helper struct
  struct RunEventNo {
    RunEventNo(const uint32_t r, const uint32_t sr, const uint32_t e, const string& remain)
      :run(r),subrun(sr),event(e),remainder(remain){}

    uint32_t run;
    uint32_t subrun;
    uint32_t event;
    string remainder;

    static RunEventNo tokenize( const string& str ) {

      // Get run/subrun/event nos. and remainder
      auto runSubrunEvtNos = getTuple<uint32_t,uint32_t,uint32_t,string>( str,
                                                                          runEventNosRML.regex_pattern,
                                                                          runEventNosRML.match_list );

      return RunEventNo( std::get<0>( runSubrunEvtNos ),
                         std::get<1>( runSubrunEvtNos ),
                         std::get<2>( runSubrunEvtNos ),
                         std::get<3>( runSubrunEvtNos ) );
    }

  };

}

//==========================================================================================================

namespace sqlite {

  using std::string;
  using std::uint32_t;
  using ntuple::Ntuple;

  template <size_t N> using name_array = sqlite::name_array<N>;

  // Helper to assign name_array's
  template<typename ... ARGS>
  auto assign(ARGS&& ... args) {
    return name_array<sizeof...(args)>{ std::forward<ARGS>(args)... };
  }

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

    std::string filename_;

    name_array<9u> msgHeaders_ ;
    name_array<2u> usrMessages_;
    name_array<5u> timeEvent_;
    name_array<7u> timeModule_;
    name_array<9u> memCheck_;

    // Kind of ugly, but necessary with current variadic template
    // rules.  A proposal has been made for the C++17 standard to do something like:
    //
    //    Ntuple<string...[6],unsigned,string...[2]> msgHeadersTable_;
    //
    // but we're not there yet.

    Ntuple<string,string,string,string,string,string,unsigned,string,string> msgHeadersTable_;
    Ntuple<sqlite_int64,string> usrMessagesTable_;

    using timeEvent_t  = Ntuple<sqlite_int64,uint32_t,uint32_t,uint32_t,double>;
    using timeModule_t = Ntuple<sqlite_int64,uint32_t,uint32_t,uint32_t,string,string,double>;
    using memCheck_t   = Ntuple<sqlite_int64,string,string,string,double,double,double,double,string>;

    std::unique_ptr<timeEvent_t>   timeEventTable_;
    std::unique_ptr<timeModule_t>  timeModuleTable_;
    std::unique_ptr<memCheck_t>    memCheckTable_;

    bool createTimeEventTable_;
    bool createTimeModuleTable_;
    bool createMemCheckTable_;
  };

  // Implementation
  //===============================================================================================================
  sqlite3Plugin::sqlite3Plugin(const fhicl::ParameterSet& pset)
    : ELdestination( pset )
    , filename_    ( pset.get<std::string>("filename") )
      // table headers
    , msgHeaders_  ( assign("Timestamp","Hostname","Hostaddress","Severity","Category","AppName","ProcessId","RunEventNo","ModuleName") )
    , usrMessages_ ( assign("HeaderId","Message") )
    , timeEvent_   ( assign("HeaderId","Run","Subrun","Event","Time") )
    , timeModule_  ( assign("HeaderId","Run","Subrun","Event","ModuleLabel","ModuleName","Time") )
    , memCheck_    ( assign("HeaderId","Type","ModuleName","ModuleLabel","Vsize","VsizeDelta","RSS","RSSDelta","Remainder") )
      // make the tables (on-demand tables made during execution of routePayload)
    , msgHeadersTable_ ( filename_, "MessageHeaders", msgHeaders_  )
    , usrMessagesTable_( filename_, "UserMessages"  , usrMessages_ )
      // flags for creating on-demand tables
    , createTimeEventTable_ ( true )
    , createTimeModuleTable_( true )
    , createMemCheckTable_  ( true )
  {}


  //===============================================================================================================
  void sqlite3Plugin::routePayload( const std::ostringstream& oss, const mf::ErrorObj& msg ) {

    const auto& xid = msg.xid();

    const string& timestamp  = mf::formatTime(msg.timestamp(),format.want(mf::service::MILLISECOND) ); // timestamp
    const string& hostname   = xid.hostname;                                 // host name
    const string& hostaddr   = xid.hostaddr;                                 // host address
    const string& severity   = xid.severity.getName();                       // severity
    const string& category   = xid.id;                                       // category
    const string& app        = xid.application;                              // application
    const long&   pid        = xid.pid;                                      // process id
    const string& runEventNo = mf::MessageDrop::instance()->runEvent;        // run/event no
    const string& modname    = xid.module;                                   // module name
    const string& usrMsg     =
      !oss.str().compare(0,2," \n") ? oss.str().erase(0,2) : oss.str(); // user-supplied msg
                                                                        // (remove leading " \n" if present)
    msgHeadersTable_.insert( timestamp,
                             hostname,
                             hostaddr,
                             severity,
                             category,
                             app,
                             pid,
                             runEventNo,
                             modname );

    const unsigned hid = msgHeadersTable_.lastRowid();

    // Check for timing or memory modules
    if ( check_regex( usrMsg, R"(^\s*(Time(Event|Module)>|MemoryCheck:)(.*))"s ) ){

      if ( check_regex(usrMsg, R"(^\s*TimeEvent>(.*))"s  ) ) {

        const auto evtInfo = RunEventNo::tokenize( usrMsg );

        if ( createTimeEventTable_ ) {
          timeEventTable_       = std::make_unique<timeEvent_t>( filename_, "TimeEvent", timeEvent_ );
          createTimeEventTable_ = false;
        }

        timeEventTable_->insert(hid,
                                evtInfo.run,
                                evtInfo.subrun,
                                evtInfo.event,
                                convertTo<double>( evtInfo.remainder )  ); // time
      }
      else if( check_regex(usrMsg, R"(^\s*TimeModule(.*))"s ) ) {

        const auto evtInfo = RunEventNo::tokenize( usrMsg );
        const auto others  = getTuple<string,string,double>( evtInfo.remainder, "[[:space:]]"s, {-1} );

        if ( createTimeModuleTable_ ) {
          timeModuleTable_       = std::make_unique<timeModule_t>( filename_, "TimeModule", timeModule_ );
          createTimeModuleTable_ = false;
        }

        timeModuleTable_->insert(hid,
                                 evtInfo.run,
                                 evtInfo.subrun,
                                 evtInfo.event,
                                 std::get<0>( others ), // module label
                                 std::get<1>( others ), // module name
                                 std::get<2>( others )  // time
                                 );

      }
      else if( check_regex(usrMsg, R"(^\s*MemoryCheck:(.*))"s) ) {

        const auto memPrefix = getTuple<string,string,string,string>( usrMsg,
                                                                      memCheckSplitRML.regex_pattern,
                                                                      memCheckSplitRML.match_list);

        const auto memSuffix = getTuple<double,double,double,double,string>( std::get<3>(memPrefix),
                                                                             memCheckSuffixRML.regex_pattern,
                                                                             memCheckSuffixRML.match_list,
                                                                             PERMISSIVE );

        if ( createMemCheckTable_ ) {
          memCheckTable_       = std::make_unique<memCheck_t>( filename_, "MemCheck", memCheck_ );
          createMemCheckTable_ = false;
        }

        memCheckTable_->insert(hid,
                               std::get<0>( memPrefix ), // type
                               std::get<1>( memPrefix ), // module name
                               std::get<2>( memPrefix ), // module label
                               std::get<0>( memSuffix ), // Vsize
                               std::get<1>( memSuffix ), // VsizeDelta
                               std::get<2>( memSuffix ), // RSS
                               std::get<3>( memSuffix ), // RSSDelta
                               std::get<4>( memSuffix )  // Remainder (can be empty)
                               );
      }
    }
    else {
      usrMessagesTable_.insert(hid,usrMsg);
    }

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


