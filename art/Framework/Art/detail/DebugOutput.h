#ifndef art_Framework_Art_detail_DebugOutput_h
#define art_Framework_Art_detail_DebugOutput_h

#include "art/Utilities/ostream_handle.h"
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
    , mode_{fhicl::detail::print_mode::raw}
    , osp_{}
  {}


  fhicl::detail::print_mode mode() const { return mode_; }

  art::ostream_handle& stream() { return *osp_; }

  void to_cerr() { dest_ = destination::cerr; }

  void set_filename(std::string const& fn)
  {
    filename_ = fn;
    dest_ = destination::file;
  }

  void set_mode(fhicl::detail::print_mode const pm){ mode_ = pm; }

  explicit operator bool() { return maybe_initialize_(); }

  static destination destination_via_env(std::string& fn) {

    char const * debug_config { getenv("ART_DEBUG_CONFIG") };
    if ( debug_config == nullptr ) return destination::none;

    bool isFilename{false};
    try {
      isFilename = std::regex_match(debug_config, std::regex("[[:alpha:]/\\.].*"));
      fn = debug_config;
    }
    catch(std::regex_error const & e) {
      std::cerr << "REGEX ERROR: " << e.code() << ".\n";
    }

    return isFilename ? destination::file : destination::cerr;

  };

private:

  destination dest_;
  fhicl::detail::print_mode mode_;
  std::string filename_;
  std::unique_ptr<art::ostream_handle> osp_;

  bool maybe_initialize_() {
    switch(dest_) {
    case destination::none : return false;
    case destination::file : {
      std::cerr << "** ART_DEBUG_CONFIG is defined: config debug output to file "
                << filename_
                << " **\n";
      osp_ = std::make_unique<art::ostream_owner>(filename_);
      if ( osp_->stream() ) break;
      std::cerr << "Output of config to " << filename_ << " failed: fallback to stderr.\n";
    }
    case destination::cerr : {
      std::cerr << "** ART_DEBUG_CONFIG is defined: config debug output follows **\n";
      osp_ = std::make_unique<art::ostream_observer>(std::cerr);
      break;
    }
    }
    return true;
  }
};

#endif

// Local variables:
// mode: c++
// End:
