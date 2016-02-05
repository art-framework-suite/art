// ======================================================================
//
// BranchType.cc
//
// ======================================================================


// framework support:
#include "canvas/Persistency/Provenance/BranchType.h"

#include <vector>

namespace art {

  namespace {

    std::string const undefinedType            = "UNDEFINED";

    // Suffixes
    std::string const auxiliary                = "Auxiliary";
    std::string const productProvenance        = "BranchEntryInfo";
    std::string const majorIndex               = ".id_.run_";
    std::string const metaData                 = "MetaData";
    std::string const productStatus            = "ProductStatus";
    std::string const statusInformation        = "StatusInformation";

    // Prefixes
    std::string const run                      = "Run";
    std::string const subRun                   = "SubRun";
    std::string const event                    = "Event";
    std::string const results                  = "Results";

    // Trees, branches, indices
    std::string const runs                     = run    + 's';
    std::string const subRuns                  = subRun + 's';
    std::string const events                   = event  + 's';
    std::string const resultsTree              = results + "Tree";

    std::string const runMeta                  = run    + metaData;
    std::string const subRunMeta               = subRun + metaData;
    std::string const eventMeta                = event  + metaData;
    std::string const resultsMeta              = results + metaData;

    std::string const runAuxiliary             = run    + auxiliary;
    std::string const subRunAuxiliary          = subRun + auxiliary;
    std::string const eventAuxiliary           = event  + auxiliary;
    std::string const resultsAuxiliary         = results + auxiliary;

    std::string const runProductProvenance        = run    + productProvenance;
    std::string const subRunProductProvenance     = subRun + productProvenance;
    std::string const eventProductProvenance      = event  + productProvenance;
    std::string const resultsProductProvenance    = results  + productProvenance;

    std::string const runMajorIndex            = runAuxiliary    + majorIndex;
    std::string const subRunMajorIndex         = subRunAuxiliary + majorIndex;
    std::string const eventMajorIndex          = eventAuxiliary  + majorIndex;

    std::string const runMinorIndex; // Empty.
    std::string const subRunMinorIndex         = subRunAuxiliary + ".id_.subRun_";
    std::string const eventMinorIndex          = eventAuxiliary  + ".id_.event_";

    inline
    std::string const & select(BranchType bt,
                               std::vector<std::string const *> const & strs)
    {
      return (static_cast<size_t>(bt) < strs.size()) ? *strs[bt] : undefinedType;
    }

  }  // namespace

  // Ordering muust match that in art/Persistency/Provenance/BranchType.h.
  std::string const & BranchTypeToString( BranchType bt ) {
    return select( bt, { &event, &subRun, &run, &results } );
  }

  std::string const & BranchTypeToProductTreeName( BranchType bt ) {
    return select( bt, { &events, &subRuns, &runs, &resultsTree } );
  }

  std::string const & BranchTypeToMetaDataTreeName( BranchType bt ) {
    return select( bt, { &eventMeta, &subRunMeta, &runMeta, &resultsMeta } );
  }

  std::string const & BranchTypeToAuxiliaryBranchName( BranchType bt ) {
    return select( bt, { &eventAuxiliary, &subRunAuxiliary, &runAuxiliary, &resultsAuxiliary } );
  }

  std::string const & productProvenanceBranchName( BranchType bt ) {
    return select( bt, { &eventProductProvenance, &subRunProductProvenance, &runProductProvenance, &resultsProductProvenance } );
  }

  std::string const & BranchTypeToMajorIndexName( BranchType bt ) {
    return select( bt, { &eventMajorIndex, &subRunMajorIndex, &runMajorIndex } );
  }

  std::string const & BranchTypeToMinorIndexName( BranchType bt ) {
    return select( bt, { &eventMinorIndex, &subRunMinorIndex, &runMinorIndex } );
  }

}  // art

// ======================================================================
