#ifndef art_Framework_Core_EDAnalyzer_h
#define art_Framework_Core_EDAnalyzer_h
// vim: set sw=2 expandtab :

//
//  The base class for all analyzer modules.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Analyzer.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>

namespace art {

  class EDAnalyzer : public detail::Analyzer {
    friend class WorkerT<EDAnalyzer>;

  public:
    using WorkerType = WorkerT<EDAnalyzer>;
    using ModuleType = EDAnalyzer;

    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::legacy;
    }

    using detail::Analyzer::Analyzer;

    std::string workerType() const;

  private:
    void doBeginJob();
  };

  class SharedAnalyzer : public detail::Analyzer {
    friend class WorkerT<SharedAnalyzer>;

  public:
    using WorkerType = WorkerT<SharedAnalyzer>;
    using ModuleType = SharedAnalyzer;

    using detail::Analyzer::Analyzer;

    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::shared;
    }

    std::string workerType() const;

  private:
    void doBeginJob();
  };

  class ReplicatedAnalyzer : public detail::Analyzer {
    friend class WorkerT<ReplicatedAnalyzer>;

  public:
    using WorkerType = WorkerT<ReplicatedAnalyzer>;
    using ModuleType = ReplicatedAnalyzer;

    using detail::Analyzer::Analyzer;

    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::replicated;
    }

    std::string workerType() const;

  private:
    void doBeginJob();
  };

} // namespace art

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
