#include "art/Framework/Core/detail/issue_reports.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <ctime>
#include <string>

namespace {
  std::string const st{"st"};
  std::string const nd{"nd"};
  std::string const rd{"rd"};
  std::string const th{"th"};

  std::string const&
  suffix(unsigned const count)
  {
    // *0, *4 - *9 use "th".
    unsigned const lastDigit = count % 10;
    if (lastDigit >= 4 || lastDigit == 0)
      return th;
    // *11, *12, or *13 use "th".
    if (count % 100 - lastDigit == 10)
      return th;
    return (lastDigit == 1 ? st : (lastDigit == 2 ? nd : rd));
  }
} // namespace

void
art::detail::issue_reports(unsigned const count, EventID const& id)
{
  time_t t = time(0);
  constexpr char tsOutput[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
  constexpr size_t tsmax = (sizeof tsOutput) + 1;
  char ts[tsmax];
  strftime(ts, tsmax, "%d-%b-%Y %H:%M:%S %Z", localtime(&t));
  mf::LogVerbatim{"ArtReport"} << "Begin processing the " << count
                               << suffix(count) << " record. " << id << " at "
                               << ts;
  // At some point we may want to initiate checkpointing here
}
