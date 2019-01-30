#ifndef art_test_Framework_Core_data_dependencies_Configs_h
#define art_test_Framework_Core_data_dependencies_Configs_h

#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/OptionalSequence.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TupleAs.h"

#include <string>

namespace art::test {

  inline void
  throwIfEmpty(std::string const& friendlyName)
  {
    if (friendlyName.empty()) {
      throw Exception{errors::Configuration,
                      "There was an error processing typenames.\n"}
        << "No friendly type name was provided.\n";
    }
  }

  // For producing products
  struct TypeAndInstance {
    explicit TypeAndInstance(std::string friendlyName, std::string instance)
      : friendlyClassName{(throwIfEmpty(friendlyName), move(friendlyName))}
      , productInstanceName{move(instance)}
    {}

    std::string const friendlyClassName;
    std::string const productInstanceName;
  };

  // For consuming products
  struct TypeAndTag {
    explicit TypeAndTag(std::string friendlyName, art::InputTag tag)
      : friendlyClassName{(throwIfEmpty(friendlyName), move(friendlyName))}
      , inputTag{std::move(tag)}
    {}

    std::string const friendlyClassName;
    art::InputTag const inputTag;
  };

  struct TopLevelTable {
    using Name = fhicl::Name;
    struct TestProperties {
      fhicl::Atom<bool> graph_failure_expected{Name{"graph_failure_expected"}};
      fhicl::OptionalAtom<std::string> error_message{Name{"error_message"}};
    };
    fhicl::Table<TestProperties> test_properties{Name{"test_properties"}};
    fhicl::Atom<std::string> process_name{Name{"process_name"}};
    struct Source {
      fhicl::Atom<std::string> module_type{Name{"module_type"}};
    };
    fhicl::OptionalTable<Source> source{Name{"source"}};
    fhicl::OptionalDelegatedParameter physics{Name{"physics"}};
    fhicl::OptionalDelegatedParameter outputs{Name{"outputs"}};
  };

  struct ModifierModuleConfig {
    using Name = fhicl::Name;
    fhicl::OptionalSequence<
      fhicl::TupleAs<TypeAndInstance(std::string, std::string)>>
      produces{Name{"produces"}};
    fhicl::OptionalSequence<
      fhicl::TupleAs<TypeAndTag(std::string, art::InputTag)>>
      consumes{Name{"consumes"}};
    fhicl::OptionalSequence<std::string> consumesMany{Name{"consumesMany"}};
  };

  struct ObserverModuleConfig {
    using Name = fhicl::Name;
    fhicl::OptionalSequence<
      fhicl::TupleAs<TypeAndTag(std::string, art::InputTag)>>
      consumes{Name{"consumes"}};
    fhicl::OptionalSequence<std::string> consumesMany{Name{"consumesMany"}};
    fhicl::OptionalSequence<std::string> select_events{Name{"SelectEvents"}};
  };
}

#endif /* art_test_Framework_Core_data_dependencies_Configs_h */

// Local Variables:
// mode: c++
// End:
