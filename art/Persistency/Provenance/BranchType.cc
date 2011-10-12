// ======================================================================
//
// BranchType.cc
//
// ======================================================================


// framework support:
#include "art/Persistency/Provenance/BranchType.h"

namespace art {

  namespace {

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

    // Trees, branches, indices
    std::string const runs                     = run    + 's';
    std::string const subRuns                  = subRun + 's';
    std::string const events                   = event  + 's';

    std::string const runMeta                  = run    + metaData;
    std::string const subRunMeta               = subRun + metaData;
    std::string const eventMeta                = event  + metaData;

    std::string const runAuxiliary             = run    + auxiliary;
    std::string const subRunAuxiliary          = subRun + auxiliary;
    std::string const eventAuxiliary           = event  + auxiliary;

    std::string const runProductProvenance        = run    + productProvenance;
    std::string const subRunProductProvenance     = subRun + productProvenance;
    std::string const eventProductProvenance      = event  + productProvenance;

    std::string const runMajorIndex            = runAuxiliary    + majorIndex;
    std::string const subRunMajorIndex         = subRunAuxiliary + majorIndex;
    std::string const eventMajorIndex          = eventAuxiliary  + majorIndex;

    std::string const runMinorIndex;           // empty
    std::string const subRunMinorIndex         = subRunAuxiliary + ".id_.subRun_";
    std::string const eventMinorIndex          = eventAuxiliary  + ".id_.event_";

    inline
    std::string const & select(BranchType          bt
                               , std::string const & event_str
                               , std::string const & run_str
                               , std::string const & subRun_str
                              )
    {
      switch (bt) {
        case InEvent :  return event_str;
        case InRun   :  return run_str;
        case InSubRun:  return subRun_str;
        default      :  return subRun_str;  // TODO: report "none of the above"?
      }
    }

  }  // namespace

  std::string const & BranchTypeToString(BranchType bt)
  {
    return select(bt, event, run, subRun);
  }

  std::string const & BranchTypeToProductTreeName(BranchType bt)
  {
    return select(bt, events, runs, subRuns);
  }

  std::string const & BranchTypeToMetaDataTreeName(BranchType bt)
  {
    return select(bt, eventMeta, runMeta, subRunMeta);
  }

  std::string const & BranchTypeToAuxiliaryBranchName(BranchType bt)
  {
    return select(bt, eventAuxiliary, runAuxiliary, subRunAuxiliary);
  }

  std::string const & productProvenanceBranchName(BranchType bt)
  {
    return select(bt, eventProductProvenance, runProductProvenance, subRunProductProvenance);
  }

  std::string const & BranchTypeToMajorIndexName(BranchType bt)
  {
    return select(bt, eventMajorIndex, runMajorIndex, subRunMajorIndex);
  }

  std::string const & BranchTypeToMinorIndexName(BranchType bt)
  {
    return select(bt, eventMinorIndex, runMinorIndex, subRunMinorIndex);
  }

}  // art

// ======================================================================
