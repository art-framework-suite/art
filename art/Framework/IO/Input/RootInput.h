#ifndef IOPool_Input_PoolInput_h
#define IOPool_Input_PoolInput_h

// ======================================================================
//
// PoolInput: This is an InputSource
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Input/Inputfwd.h"
#include "art/Framework/IO/Sources/EDInputSource.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "boost/array.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {
  class RootInputFileSequence;
  class FileCatalogItem;

  class PoolInput;
}

class art::PoolInput
  : public EDInputSource
{
public:
  explicit PoolInput( fhicl::ParameterSet    const & pset
                     , InputSourceDescription const & desc
                     );
  virtual ~PoolInput( );

  using InputSource::productRegistryUpdate;
  using InputSource::runPrincipal;

private:
  boost::scoped_ptr< RootInputFileSequence >             primaryFileSequence_;
  boost::scoped_ptr< RootInputFileSequence >             secondaryFileSequence_;
  boost::array< std::vector<BranchID>, NumBranchTypes >  branchIDsToReplace_;

  typedef  boost::shared_ptr<RootInputFile>  RootInputFileSharedPtr;
  typedef  input::EntryNumber           EntryNumber;

  virtual std::auto_ptr<EventPrincipal>
    readEvent_( );
  virtual boost::shared_ptr<SubRunPrincipal>
    readSubRun_( );
  virtual boost::shared_ptr<RunPrincipal>
    readRun_( );
  virtual boost::shared_ptr<FileBlock>
    readFile_( );
  virtual void
    closeFile_( );
  virtual void
    endJob( );
  virtual ItemType
    getNextItemType( );
  virtual std::auto_ptr<EventPrincipal>
    readIt( EventID const & id );
  virtual void
    skip( int offset );
  virtual void
    rewind_( );
}; // PoolInput

// ======================================================================

#endif
