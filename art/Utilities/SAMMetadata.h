#ifndef art_Utilities_SAMMetadata_h
#define art_Utilities_SAMMetadata_h

#include "cetlib/container_algorithms.h"

#include <iostream>
#include <map>
#include <string>

namespace art {

  using new_t = std::string;
  using old_t = std::string;

  inline std::map<new_t,old_t> newToOldName()
  {
    return {
      { "file_type", "fileType" },
      { "data_tier", "dataTier" },
      { "data_stream", "streamName" },
      { "run_type", "runType" }
    };
  }

  inline std::map<new_t,old_t> oldToNewName()
  {
    auto const newToOld = newToOldName();
    std::map<old_t,new_t> oldToNew;
    cet::transform_all(newToOld,
                       std::inserter(oldToNew, oldToNew.begin()),
                       [](auto const& pr){ return std::make_pair(pr.second, pr.first); } );

    return oldToNew;
  }

  inline bool is_new_md_name(std::string const & newMD)
  {
    auto const& translator = newToOldName();
    return translator.find(newMD) != translator.cend();
  }

  inline bool is_old_md_name(std::string const & newMD)
  {
    auto const& translator = oldToNewName();
    return translator.find(newMD) != translator.cend();
  }

  inline std::string old_md_name(std::string const & newMD)
  {
    if (is_old_md_name(newMD)) {
      throw art::Exception(art::errors::LogicError)
        << "Asked for nonexistent new -> old MD translation for MD item "
        << newMD
        << ".\n";
    }
    return newToOldName()[newMD];
  }

 inline std::string new_md_name(std::string const & oldMD)
  {
    if (is_new_md_name(oldMD)) {
      throw art::Exception(art::errors::LogicError)
        << "Asked for nonexistent old -> new MD translation for MD item "
        << oldMD
        << ".\n";
    }
    return oldToNewName()[oldMD];
  }

}

#endif

// Local variables:
// mode: c++
// End:
