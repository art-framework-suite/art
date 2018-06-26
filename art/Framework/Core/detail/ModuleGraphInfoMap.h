#ifndef art_Framework_Core_detail_ModuleGraphInfoMap_h
#define art_Framework_Core_detail_ModuleGraphInfoMap_h

#include "art/Framework/Core/detail/ModuleGraphInfo.h"
#include "art/Framework/Core/detail/graph_type_aliases.h"

namespace art {
  namespace detail {

    class ModuleGraphInfoMap {
    public:
      explicit ModuleGraphInfoMap(collection_map_t const& paths);

      auto const&
      name(std::size_t const i) const
      {
        return modules_[i].first;
      }

      auto const&
      info(std::size_t const i) const
      {
        return modules_[i].second;
      }

      auto
      size() const
      {
        return modules_.size();
      }

      auto
      begin() const
      {
        return modules_.begin();
      }

      auto
      end() const
      {
        return modules_.end();
      }

      auto info(module_name_t const& name) const -> ModuleGraphInfo const&;
      auto vertex_index(module_name_t const& name) const -> distance_t;

    private:
      collection_t const modules_;
      collection_t::const_iterator const begin_;
      collection_t::const_iterator const end_;
    };
  }
}

#endif /* art_Framework_Core_detail_ModuleGraphInfoMap_h */

// Local Variables:
// mode: c++
// End:
