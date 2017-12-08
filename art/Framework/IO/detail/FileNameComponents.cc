#include "art/Framework/IO/detail/FileNameComponents.h"
#include "boost/regex.hpp"

#include <cassert>
#include <iomanip>
#include <sstream>

namespace art {
  namespace detail {

    void
    FileNameComponents::add(std::string const& prefix,
                            std::string const& digitFormat)
    {
      components_.emplace_back(prefix, digitFormat);
    }

    void
    FileNameComponents::setSuffix(std::string suffix)
    {
      suffix_ = move(suffix);
    }

    bool
    FileNameComponents::operator<(FileNameComponents const& fnc) const
    {
      if (components_.size() < fnc.components_.size()) {
        return true;
      }
      if (components_.size() > fnc.components_.size()) {
        return false;
      }

      assert(components_.size() == fnc.components_.size());
      auto b1 = cbegin(components_), e1 = cend(components_);
      auto b2 = cbegin(fnc.components_);
      for (; b1 != e1; ++b1, ++b2) {
        if (b1->first < b2->first) return true;
      }
      assert(b2 == cend(fnc.components_));
      return false;
    }

    std::string
    FileNameComponents::fileNameWithIndex(std::size_t const index) const
    {
      std::ostringstream num_str;
      for (auto const& pr : components_) {
        auto const& prefix = pr.first;
        auto const& specifier = pr.second;
        num_str << prefix;
        if (!specifier.empty()) { // Zero-filling.
          num_str << std::setfill('0') << std::setw(std::stoul(specifier));
        }
        num_str << index;
      }
      num_str << suffix_;
      return num_str.str();
    }

    FileNameComponents
    componentsFromPattern(std::string const& pattern)
    {
      FileNameComponents result;

      boost::smatch match;
      auto si = cbegin(pattern), se = cend(pattern);
      while (boost::regex_search(si, se, match, boost::regex{"%(\\d+)?#", boost::regex_constants::ECMAScript})) {
        assert(match.size() == 2);
        // Subexpressions:
        //   0. Entire matched expression
        //   1. Possible fill format digits for numeric substitution.
        result.add(match.prefix(), match[1].str());
        si = match[0].second;
      }
      // Get remaining characters of filename
      result.setSuffix(std::string(si, se));
      return result;
    }

  }
}
