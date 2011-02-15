// ======================================================================
//
// FileDumperOutput.cc: "dump contents of a file"
//
// Proposed output format (Feature #941):
// Process Name | Module Label | Process Label | Data Product type | size
//
// ======================================================================

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "cetlib/column_width.h"
#include "cetlib/lpad.h"
#include "cetlib/rpad.h"
#include "fhiclcpp/ParameterSet.h"
#include <algorithm>
#include <iostream>

namespace art {
  class FileDumperOutput;
}

using art::FileDumperOutput;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

class art::FileDumperOutput
  : public OutputModule
{
public:
  explicit FileDumperOutput( ParameterSet const & );

  virtual ~FileDumperOutput( ) { }

private:
  virtual void
    write      ( EventPrincipal const & e );
  virtual void
    writeRun   ( RunPrincipal const & r );
  virtual void
    writeSubRun( SubRunPrincipal const & sr );

};  // FileDumperOutput

// ----------------------------------------------------------------------

  FileDumperOutput::FileDumperOutput( ParameterSet const & ps )
: OutputModule( ps )
{ }

// ----------------------------------------------------------------------

void
  FileDumperOutput::write( EventPrincipal const & e )
{
  typedef  std::vector<std::string>  column;
  unsigned int ncols = 5;
  std::vector<column> col(ncols);
  col[0].push_back("PROCESS NAME");
  col[1].push_back("MODULE LABEL");
  col[2].push_back("PRODUCT INSTANCE NAME");
  col[3].push_back("DATA PRODUCT TYPE");
  col[4].push_back("SIZE");

  for( EventPrincipal::const_iterator it  = e.begin()
                                    , end = e.end(); it != end; ++it ) {
    Group const & g = *(it->second);

    col[0].push_back( g.processName() );
    col[1].push_back( g.moduleLabel() );
    col[2].push_back( g.productInstanceName() );
    col[3].push_back( g.productType().Name( Reflex::FINAL
                                          | Reflex::SCOPED
                                          | Reflex::QUALIFIED
                    )                     );
    col[4].push_back( g.product()->productSize() );
  }

  std::vector<unsigned> width(ncols);
  std::transform( col.begin(), col.end()
                , width.begin()
                , cet::column_width
                );

  for( unsigned row = 0, end = col[0].size(); row != end; ++row ) {
    std::string s;
    for( unsigned c = 0, end = ncols-1; c != end; ++c ) {
      s.append( cet::rpad(col[c][row], width[c], '.') )
       .append( " | ");
    }
    s.append( cet::lpad(col[ncols-1][row], width[ncols-1], '.') );
    std::cout << s << '\n';
  }

}

// ----------------------------------------------------------------------

void
  FileDumperOutput::writeRun( RunPrincipal const & e )
{
  // required by base class, but nothing to be done here
}

// ----------------------------------------------------------------------

void
  FileDumperOutput::writeSubRun( SubRunPrincipal const & e )
{
  // required by base class, but nothing to be done here
}

// ======================================================================

DEFINE_ART_MODULE(FileDumperOutput);

// ======================================================================
