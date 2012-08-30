#include "art/Framework/Art/OptionsHandler.h"

#include "cetlib/exception.h"
#include "cetlib/demangle.h"

#include <stdexcept>
#include <typeinfo>

namespace {
  int exceptionCatcher(std::function<int ()> func,
                       std::string const & funcName __attribute__((unused)) ,
                       int failureCode)
  {
    try {
      return func();
    }
    catch (cet::exception & e) {
      std::cerr << "OptionsHandler caught a cet::exception calling "
                << funcName
                << "\n"
                << e.what();
    }
    catch (std::exception & e) {
      std::cerr << "OptionsHandler caught an std::exception calling "
                << funcName
                << "\n"
                << e.what();
    }
    catch (std::string & s) {
      std::cerr << "OptionsHandler caught a string exception calling "
                << funcName
                << "\n"
                << s;
    }
    catch (char const * s) {
      std::cerr << "OptionsHandler caught a string exception calling "
                << funcName
                << "\n"
                << s;
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
  std::string const thisClass(cet::demangle(typeid(this).name()));
  return exceptionCatcher(std::bind(&art::OptionsHandler::doCheckOptions,
                                    this,
                                    std::cref(vm)),
                          thisClass + "::doCheckOptions()",
                          7001);
}

int
art::OptionsHandler::
processOptions(bpo::variables_map const & vm,
               fhicl::intermediate_table & raw_config)
{
  std::string const thisClass(cet::demangle(typeid(this).name()));
  return exceptionCatcher(std::bind(&art::OptionsHandler::doProcessOptions,
                                    this,
                                    std::cref(vm),
                                    std::ref(raw_config)),
                          thisClass + "::doProcessOptions()",
                          7002);
}
