#ifndef art_Framework_Art_detail_DebugOutput_h
#define art_Framework_Art_detail_DebugOutput_h

#include "cetlib/ostream_handle.h"
#include "fhiclcpp/detail/print_mode.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <string>

namespace art {
  namespace detail {
    class DebugOutput;
  }
}

class art::detail::DebugOutput {
public:

  enum class destination { none, cerr, file };

  DebugOutput()
    : dest_{destination::none}
    , preempting_{true}
    , mode_{fhicl::detail::print_mode::raw}
    , osp_{}
  {}


  std::string const& filename() { return filename_; }

  fhicl::detail::print_mode mode() const { return mode_; }

  std::string banner() const {
    std::string result = "** Config output ";
    result += filename_.empty() ? "follows" : std::string("to file '"+filename_+"'");
    result += " **\n";
    return result;
  }

  cet::ostream_handle& stream() {
    return *osp_;
  }

  bool stream_is_valid(){ return osp_->stream(); }

  void to_cerr() { dest_ = destination::cerr; }

  void set_preempting(bool const p){ preempting_ = p; }

  void set_filename(std::string const& fn)
  {
    filename_ = fn;
    dest_ = destination::file;
  }

  void set_mode(fhicl::detail::print_mode const pm){ mode_ = pm; }

  bool preempting() const { return preempting_; }

  explicit operator bool() { return maybe_initialize_(); }

  static destination destination_via_env(std::string& fn) {

    char const * debug_config { getenv("ART_DEBUG_CONFIG") };
    if ( debug_config == nullptr ) return destination::none;

    bool isFilename{false};
    try {
      isFilename = std::regex_match(debug_config, std::regex("[[:alpha:]/\\.].*"));

      std::string suffix = "follows";
      if ( isFilename ) {
        fn     = debug_config;
        suffix = "to file "+fn;
      }

      std::cerr << "** ART_DEBUG_CONFIG is defined **\n";

    }
    catch(std::regex_error const & e) {
      std::cerr << "REGEX ERROR: " << e.code() << ".\n";
    }

    return isFilename ? destination::file : destination::cerr;

  };

private:

  destination dest_;
  bool preempting_;
  fhicl::detail::print_mode mode_;
  std::string filename_;
  std::unique_ptr<cet::ostream_handle> osp_;

  bool maybe_initialize_() {
    switch(dest_) {
    case destination::none : return false;
    case destination::file : {
      osp_ = std::make_unique<cet::ostream_owner>(filename_);
      if ( osp_->stream() ) break;
      std::cerr << "Output of config to " << filename_ << " failed: fallback to stderr.\n";
    }
    case destination::cerr : {
      osp_ = std::make_unique<cet::ostream_observer>(std::cerr);
      break;
    }
    }
    return true;
  }
};

#endif /* art_Framework_Art_detail_DebugOutput_h */

// Local variables:
// mode: c++
// End:
