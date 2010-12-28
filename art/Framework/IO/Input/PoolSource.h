#ifndef IOPool_Input_PoolSource_h
#define IOPool_Input_PoolSource_h

// ======================================================================
//
// PoolSource: This is an InputSource
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Input/Inputfwd.h"
#include "art/Framework/IO/Sources/VectorInputSource.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "boost/array.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include <memory>  // auto_ptr
#include <string>
#include <vector>

namespace art {
  class RootInputFileSequence;
  class FileCatalogItem;

  class PoolSource;
}

class art::PoolSource
  : public VectorInputSource
{
public:
  explicit PoolSource( fhicl::ParameterSet    const & pset
                     , InputSourceDescription const & desc
                     );
  virtual ~PoolSource( );

  using InputSource::productRegistryUpdate;
  using InputSource::runPrincipal;

private:
  boost::scoped_ptr< RootInputFileSequence >             primaryFileSequence_;
  boost::scoped_ptr< RootInputFileSequence >             secondaryFileSequence_;
  boost::array< std::vector<BranchID>, NumBranchTypes >  branchIDsToReplace_;

  typedef  boost::shared_ptr<RootFile>  RootFileSharedPtr;
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
  virtual void
    readMany_( int number
             , EventPrincipalVector & result
             );
  virtual void
    readMany_( int number
             , EventPrincipalVector & result
             , EventID const & id
             , unsigned int fileSeqNumber
             );
  virtual void
    readManyRandom_( int number
                   , EventPrincipalVector & result
                   , unsigned int & fileSeqNumber
                   );
  virtual void
    dropUnwantedBranches_( std::vector<std::string> const & wantedBranches );

}; // PoolSource

// ======================================================================

#endif  // IOPool_Input_PoolSource_h
