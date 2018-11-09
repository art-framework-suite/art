#include "art/Framework/Services/Registry/detail/ensure_only_one_thread.h"
#include "art/Utilities/Globals.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

namespace {
  std::string
  maybe_s(unsigned const i, std::string const& word)
  {
    return i == 1u ? word : word + "s";
  }
}

void
art::detail::ensure_only_one_thread(fhicl::ParameterSet const& service_pset)
{
  auto const& globals = *Globals::instance();
  auto const nschedules = globals.nschedules();
  auto const nthreads = globals.nthreads();
  if (nschedules == 1u && nthreads == 1u)
    return;

  auto const service_type = service_pset.get<std::string>("service_type");
  Exception e{errors::Configuration};
  e << "The service '" << service_type << '\'';
  std::string provider;
  if (service_pset.get_if_present("service_provider", provider)) {
    e << " (provider: '" << provider << "')";
  }
  e << " is a legacy service,\n"
    << "which can be used with only one schedule and one thread.\n"
    << "This job uses " << nschedules << maybe_s(nschedules, " schedule")
    << " and " << nthreads << maybe_s(nthreads, " thread") << ".\n"
    << "Please reconfigure your job to use only one schedule/thread.\n";
  throw e;
}
