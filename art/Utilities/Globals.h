#ifndef art_Utilities_Globals_h
#define art_Utilities_Globals_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/TypeID.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <array>
#include <atomic>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace art {

  class EventProcessor;
  class Scheduler;

  class ProductInfo {
  public: // TYPES
    enum class ConsumableType { Product = 0, ViewElement = 1, Many = 2 };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ProductInfo();
    explicit ProductInfo(ConsumableType const, TypeID const&);
    explicit ProductInfo(ConsumableType const,
                         TypeID const&,
                         std::string const& label,
                         std::string const& instance,
                         std::string const& process);

  public: // MEMBER DATA -- FIXME: Are these supposed to be public?
    // FIXME: We need a way to tell whether this came from consumes or from may
    // consume!!!

    // Which kind of the DataViewImpl::get* functions we validate.
    ConsumableType consumableType_{};

    // Data product class type.
    // Part 1 of branch name.
    TypeID typeID_;

    // Note: This part is only provided and used by the DataViewImpl::get*
    // functions. Data product module label. Part 2 of branch name.
    std::string label_{};

    // Note: This part is only provided and used by the DataViewImpl::get*
    // functions. Data product instance name. Part 3 of branch name.
    std::string instance_{};

    // Note: This part is only provided and used by the DataViewImpl::get*
    // functions. Data product process name. Part 4 of branch name.
    std::string process_{};
  };

  bool operator<(ProductInfo const& a, ProductInfo const& b);
  std::ostream& operator<<(std::ostream& os,
                           ProductInfo::ConsumableType const ct);
  std::ostream& operator<<(std::ostream& os, ProductInfo const& info);

  class ConsumesInfo {
  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ConsumesInfo();
    ConsumesInfo(ConsumesInfo const&) = delete;
    ConsumesInfo(ConsumesInfo&&) = delete;
    ConsumesInfo& operator=(ConsumesInfo const&) = delete;
    ConsumesInfo& operator=(ConsumesInfo&&) = delete;

  private: // MEMBER FUNCTIONS -- Special Member Functions
    ConsumesInfo();

  public: // MEMBER FUNCTIONS -- Static API
    static ConsumesInfo* instance();
    static std::string assemble_consumes_statement(BranchType const,
                                                   ProductInfo const&);
    static std::string module_context(ModuleDescription const&);

  public: // MEMBER FUNCTIONS -- API for user
    void setRequireConsumes(bool const);

    // Maps module label to run, per-branch consumes info.
    using consumables_t =
      std::map<std::string const,
               std::array<std::vector<ProductInfo>, NumBranchTypes>>;

    std::array<std::vector<ProductInfo>, NumBranchTypes> const&
    consumables(std::string const& module_label) const
    {
      return consumables_.at(module_label);
    }

    void collectConsumes(
      std::string const& module_label,
      std::array<std::vector<ProductInfo>, NumBranchTypes> const& consumables);

    // This is used by get*() in DataViewImpl.
    void validateConsumedProduct(BranchType const,
                                 ModuleDescription const&,
                                 ProductInfo const& productInfo);

    void showMissingConsumes() const;

  private: // MEMBER DATA
    // Protects access to consumables_ and missingConsumes_.
    mutable hep::concurrency::RecursiveMutex mutex_{
      "art::ConsumesInfo::mutex_"};

    std::atomic<bool> requireConsumes_;

    // Maps module label to run, per-branch consumes info.
    std::map<std::string const,
             std::array<std::vector<ProductInfo>, NumBranchTypes>>
      consumables_;

    // Maps module label to run, per-branch missing product consumes info.
    std::map<std::string const,
             std::array<std::set<ProductInfo>, NumBranchTypes>>
      missingConsumes_;
  };

  class Globals {

    friend class EventProcessor;
    friend class Scheduler;

    // MEMBER FUNCTIONS -- Special Member Functions
  public:
    ~Globals();
    Globals(Globals const&) = delete;
    Globals(Globals&&) = delete;
    Globals& operator=(Globals const&) = delete;
    Globals& operator=(Globals&&) = delete;

    // MEMBER FUNCTIONS -- Special Member Functions
  private:
    Globals();

    // MEMBER FUNCTIONS -- Static API
  public:
    static Globals* instance();

    // MEMBER FUNCTIONS -- API for getting system-wide settings
  public:
    int nschedules() const;
    int nthreads() const;
    std::string const& processName() const;
    fhicl::ParameterSet const& triggerPSet() const;
    std::vector<std::string> const& triggerPathNames() const;

    // MEMBER FUNCTIONS -- API for setting system-wide settings, only for
    // friends
  private:
    void setNSchedules(int);
    void setNThreads(int);
    void setProcessName(std::string const&);
    void setTriggerPSet(fhicl::ParameterSet const&);
    void setTriggerPathNames(std::vector<std::string> const&);

    // MEMBER DATA
  private:
    // The services.scheduler.nschedules parameter.
    int nschedules_{1};

    // The services.scheduler.nthreads parameter.
    int nthreads_{1};

    // The art process_name from the job pset.
    std::string processName_;

    // Parameter set of trigger paths, the key is "trigger_paths",
    // and the value is triggerPathNames_.
    fhicl::ParameterSet triggerPSet_;

    // Trigger path names, passed to ctor.
    std::vector<std::string> triggerPathNames_;
  };

} // namespace art

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Utilities_Globals_h */
