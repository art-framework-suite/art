#ifndef art_Framework_Art_detail_DebugOutput_h
#define art_Framework_Art_detail_DebugOutput_h

#include "canvas/Utilities/Exception.h"
#include "cetlib/compiler_macros.h"
#include "cetlib/ostream_handle.h"
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
    validate_config,
    data_dependency_graph
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

  bool
  to_cerr() const
  {
    return dest_ == destination::cerr;
  }

  void
  set_to_cerr()
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

  bool
  data_dependency_graph() const
  {
    return processing_mode_ == processing_mode::data_dependency_graph;
  }

  explicit operator bool()
  {
    return processing_mode_ != processing_mode::none &&
           dest_ != destination::none;
  }

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
};

#endif /* art_Framework_Art_detail_DebugOutput_h */

// Local variables:
// mode: c++
// End:
