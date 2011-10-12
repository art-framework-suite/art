#ifndef art_Framework_IO_Root_rootNames_h
#define art_Framework_IO_Root_rootNames_h

#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/ParameterSetMap.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"

#include <typeinfo>

namespace art {
  class FileFormatVersion;
  class FileIndex;
  class History;

  namespace rootNames {
    //------------------------------------------------------------------
    // Parentage Tree
    std::string const & parentageTreeName( );

    // Branches on parentage tree
    std::string const & parentageIDBranchName( );
    std::string const & parentageBranchName( );

    //------------------------------------------------------------------
    // MetaData Tree (1 entry per file)
    std::string const & metaDataTreeName( );

    // Event History Tree
    std::string const & eventHistoryTreeName( );

    // Branches on EventHistory Tree
    std::string const & eventHistoryBranchName( );

    //------------------------------------------------------------------
    // Other tree names
    std::string const & eventTreeName( );
    std::string const & eventMetaDataTreeName( );

#define ART_ROOTNAME(T,N)                                     \
    template <>                                               \
    inline char const *metaBranchRootName<T>() { return N; }

#define ART_ROOTNAME_SIMPLE(T)                  \
    ART_ROOTNAME(T,#T)

    template <typename T>
    char const *metaBranchRootName() {
      throw Exception(errors::LogicError)
        << "art::metaBranchRootName requires a specialization for type "
        << TypeID(typeid(T)).className()
        << "\n";
    }

    ART_ROOTNAME_SIMPLE(FileFormatVersion)
    ART_ROOTNAME_SIMPLE(FileIndex)
    ART_ROOTNAME(ProductRegistry, "ProductRegistry")
    ART_ROOTNAME_SIMPLE(ParameterSetMap)
    ART_ROOTNAME_SIMPLE(ProcessHistoryMap)
    ART_ROOTNAME_SIMPLE(BranchIDLists)
    ART_ROOTNAME(BranchChildren,"ProductDependencies")
    ART_ROOTNAME(History,"EventHistory")

#undef ART_ROOTNAME_SIMPLE
#undef ART_ROOTNAME

  }  // rootNames
}
#endif /* art_Framework_IO_Root_rootNames_h */

// Local Variables:
// mode: c++
// End:
