#ifndef art_Framework_Core_detail_ModuleInPathInfo_h
#define art_Framework_Core_detail_ModuleInPathInfo_h

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "cetlib/exempt_ptr.h"

namespace art {
  namespace detail {
    class ModuleInPathInfo;
  }
}

class art::detail::ModuleInPathInfo {
public:
  ModuleInPathInfo(ModuleConfigInfo const& mci,
                   WorkerInPath::FilterAction filterAction);

  ModuleConfigInfo const& moduleConfigInfo() const;
  WorkerInPath::FilterAction filterAction() const;

private:
  cet::exempt_ptr<ModuleConfigInfo const> mci_;
  WorkerInPath::FilterAction filterAction_;
};

inline art::detail::ModuleInPathInfo::ModuleInPathInfo(
  ModuleConfigInfo const& mci,
  WorkerInPath::FilterAction filterAction)
  : mci_(&mci), filterAction_(filterAction)
{}

inline art::detail::ModuleConfigInfo const&
art::detail::ModuleInPathInfo::moduleConfigInfo() const
{
  return *mci_;
}

inline art::WorkerInPath::FilterAction
art::detail::ModuleInPathInfo::filterAction() const
{
  return filterAction_;
}
#endif /* art_Framework_Core_detail_ModuleInPathInfo_h */

// Local Variables:
// mode: c++
// End:
