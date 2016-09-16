#include "art/Framework/Art/OptionsHandler.h"

#include "cetlib/exception.h"
#include "cetlib/demangle.h"

#include <iostream>
#include <stdexcept>
#include <typeinfo>

namespace {

  // class F must be callable with no arguments, and return 'int'.
  template <class F>
  int exceptionCatcher(F func,
                       std::string const& funcName [[gnu::unused]],
                       int failureCode)
  {
    try {
      return func();
    }
    catch (cet::exception & e) {
      std::cerr << "OptionsHandler caught a cet::exception calling "
                << funcName
                << "\n"
                << e.what()
                << "\n";
    }
    catch (std::exception & e) {
      std::cerr << "OptionsHandler caught an std::exception calling "
                << funcName
                << "\n"
                << e.what()
                << "\n";
    }
    catch (std::string & s) {
      std::cerr << "OptionsHandler caught a string exception calling "
                << funcName
                << "\n"
                << s
                << "\n";
    }
    catch (char const * s) {
      std::cerr << "OptionsHandler caught a string exception calling "
                << funcName
                << "\n"
                << s
                << "\n";
    }
    catch (...) {
      std::cerr << "OptionsHandler caught an unknown exception calling "
                << funcName
                << "\n";
    }
    return failureCode;
  }
}

int
art::OptionsHandler::
checkOptions(bpo::variables_map const & vm)
{
  std::string const thisClass(cet::demangle_symbol(typeid(*this).name()));
  return exceptionCatcher([&vm,this](){ return this->doCheckOptions(vm); },
                          thisClass + "::doCheckOptions()",
                          89);
}

int
art::OptionsHandler::
processOptions(bpo::variables_map const & vm,
               fhicl::intermediate_table & raw_config)
{
  std::string const thisClass(cet::demangle_symbol(typeid(*this).name()));
  return exceptionCatcher([&, this](){
      return this->doProcessOptions(vm, raw_config);
    },
    thisClass + "::doProcessOptions()",
    90);
}
