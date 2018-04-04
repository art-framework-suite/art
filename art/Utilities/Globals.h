#ifndef art_Utilities_Globals_h
#define art_Utilities_Globals_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/TypeID.h"

#include <array>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace art {

  class EventProcessor;

  class ProductInfo {

  public: // TYPES
    enum class ConsumableType {
      Product // 0
      ,
      ViewElement // 1
      ,
      Many // 2
    };

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

  inline std::ostream&
  operator<<(std::ostream& os, ProductInfo::ConsumableType const ct)
  {
    switch (ct) {
      case ProductInfo::ConsumableType::Product:
        os << "Product";
        break;
      case ProductInfo::ConsumableType::ViewElement:
        os << "ViewElement";
        break;
      case ProductInfo::ConsumableType::Many:
        os << "Many";
        break;
    }
    return os;
  }

  std::ostream& operator<<(std::ostream& os, ProductInfo const& info);

  // using ConsumableProductVectorPerBranch = std::vector<ProductInfo>;
  // using ConsumableProductSetPerBranch = std::set<ProductInfo>;
  // using ConsumableProducts = std::array<std::vector<ProductInfo>,
  // NumBranchTypes>;  using ConsumableProductSets =
  // std::array<std::set<ProductInfo>, NumBranchTypes>;

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

    consumables_t::mapped_type const&
    consumables(std::string const& module_label) const
    {
      // Will throw if entry for module label doesn't exist.
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
    bool requireConsumes_{};

    consumables_t consumables_{};

    // Maps module label to run, per-branch missing product consumes info.
    std::map<std::string const,
             std::array<std::set<ProductInfo>, NumBranchTypes>>
      missingConsumes_{};
  };

  class Globals {

    friend class EventProcessor;
    friend class PathManager;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~Globals();
    Globals(Globals const&) = delete;
    Globals(Globals&) = delete;
    Globals& operator=(Globals const&) = delete;
    Globals& operator=(Globals&) = delete;

  private: // MEMBER FUNCTIONS -- Special Member Functions
    Globals();

  public: // MEMBER FUNCTIONS -- Static API
    static Globals* instance();

  public: // MEMBER FUNCTIONS -- API for getting system-wide settings
    int nthreads();
    int nschedules();

  private: // MEMBER FUNCTIONS -- API for setting system-wide settings, only for
           // friends
    void setNThreads(int);
    void setNSchedules(int);

  private: // MEMBER DATA
    int nthreads_{1};
    int nschedules_{1};
  };

} // namespace art

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Utilities_Globals_h */
