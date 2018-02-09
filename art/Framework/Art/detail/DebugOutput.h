#ifndef art_Framework_Art_detail_DebugOutput_h
#define art_Framework_Art_detail_DebugOutput_h

#include "cetlib/compiler_macros.h"
#include "cetlib/ostream_handle.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/detail/print_mode.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

namespace art {
  namespace detail {
    class DebugOutput;
  }
} // namespace art

class art::detail::DebugOutput {
public:
  enum class processing_mode {
    none,
    config_out,
    debug_config,
    validate_config
  };
  enum class destination { none, cerr, file };

  std::string const&
  filename() const
  {
    return filename_;
  }

  fhicl::detail::print_mode
  print_mode() const
  {
    return mode_;
  }

  std::string
  banner() const
  {
    std::string result = "** Config output ";
    result += filename_.empty() ? "follows" :
                                  std::string("to file '" + filename_ + "'");
    result += " **\n";
    return result;
  }

  cet::ostream_handle&
  stream()
  {
    return *osp_;
  }

  bool
  stream_is_valid()
  {
    return static_cast<bool>(*osp_);
  }

  void
  to_cerr()
  {
    dest_ = destination::cerr;
  }

  void
  set_processing_mode(processing_mode const mode)
  {
    processing_mode_ = mode;
  }

  void
  set_filename(std::string const& fn)
  {
    filename_ = fn;
    dest_ = destination::file;
  }

  void
  set_print_mode(fhicl::detail::print_mode const pm)
  {
    mode_ = pm;
  }

  bool
  validate_config() const
  {
    return processing_mode_ == processing_mode::validate_config;
  }

  bool
  debug_config() const
  {
    return processing_mode_ == processing_mode::debug_config;
  }

  bool
  config_out() const
  {
    return processing_mode_ == processing_mode::config_out;
  }

  explicit operator bool() { return maybe_initialize_(); }

  static destination
  destination_via_env(std::string& fn)
  {
    char const* debug_config{getenv("ART_DEBUG_CONFIG")};
    if (debug_config == nullptr)
      return destination::none;

    auto dest = destination::cerr;
    try {
      // Check if the provided character string is a file name
      if (std::regex_match(debug_config, std::regex("[[:alpha:]/\\.].*"))) {
        fn = debug_config;
        dest = destination::file;
      }
      std::cerr << "** ART_DEBUG_CONFIG is defined **\n";
    }
    catch (std::regex_error const& e) {
      std::cerr << "REGEX ERROR: " << e.code() << ".\n";
    }

    return dest;
  }

private:
  destination dest_{destination::none};
  processing_mode processing_mode_{processing_mode::none};
  fhicl::detail::print_mode mode_{fhicl::detail::print_mode::raw};
  std::string filename_{};
  std::unique_ptr<cet::ostream_handle> osp_{nullptr};

  bool
  maybe_initialize_()
  {
    switch (dest_) {
      case destination::none:
        return false;
      case destination::file: {
        osp_ = std::make_unique<cet::ostream_handle>(filename_);
        if (*osp_)
          break;
        std::cerr << "Output of config to " << filename_
                  << " failed: fallback to stderr.\n";
      }
      FALLTHROUGH;
      case destination::cerr: {
        osp_ = std::make_unique<cet::ostream_handle>(std::cerr);
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
