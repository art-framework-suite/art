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
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/Exception.h"
#include "cetlib/column_width.h"
#include "cetlib/lpad.h"
#include "cetlib/rpad.h"
#include "cpp0x/algorithm"
#include "fhiclcpp/ParameterSet.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

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

  template <typename P>
  void
  printPrincipal(P const & p);

  bool wantOnDemandProduction;

};  // FileDumperOutput

// ----------------------------------------------------------------------

  FileDumperOutput::FileDumperOutput( ParameterSet const & ps )
: OutputModule          ( ps )
, wantOnDemandProduction( ps.get<bool>("onDemandProduction", false) )
{ }

// ----------------------------------------------------------------------

void
  FileDumperOutput::write( EventPrincipal const & e )
{
  printPrincipal(e);
}

// ----------------------------------------------------------------------

void
  FileDumperOutput::writeRun( RunPrincipal const & r )
{
  printPrincipal(r);
}

// ----------------------------------------------------------------------

void
  FileDumperOutput::writeSubRun( SubRunPrincipal const & sr )
{
  printPrincipal(sr);
}

template <typename P>
void
FileDumperOutput::printPrincipal(P const & p) {
  if (!p.size()) return; // Nothing to do.

  // prepare the data structure, a sequence of columns:
  typedef  std::vector<std::string>  column;
  unsigned int ncols = 5;
  std::vector<column> col(ncols);

  // provide column headings:
  col[0].push_back("PROCESS NAME");
  col[1].push_back("MODULE LABEL");
  col[2].push_back("PRODUCT INSTANCE NAME");
  col[3].push_back("DATA PRODUCT TYPE");
  col[4].push_back("SIZE");

  size_t present = 0;
  size_t not_present = 0;
  // insert the per-product data:
  for(typename P::const_iterator
        it  = p.begin(),
        end = p.end();
      it != end;
      ++it ) {
    Group const & g = *(it->second);
    try {
      if (!g.resolveProduct(wantOnDemandProduction, g.producedWrapperType()))
        throw Exception(errors::DataCorruption, "data corruption");
      ++present;
    }
    catch( art::Exception const & e ) {
      if( e.category() != "ProductNotFound" )
        throw;
      if( g.anyProduct() )
        throw art::Exception(errors::LogicError, "FileDumperOutput module", e)
          << "Product reported as not present, but is pointed to nonetheless!";
      ++not_present;
    }

    col[0].push_back( g.processName() );
    col[1].push_back( g.moduleLabel() );
    col[2].push_back( g.productInstanceName() );
    col[3].push_back( g.productDescription().producedClassName() );

    if( g.anyProduct() ) {
      col[4].push_back( g.anyProduct()->productSize() );
    }
    else {
      col[4].push_back( g.onDemand() ? "o/d" : "?");
    }
  }

  // determine each column's width:
  std::vector<unsigned> width(ncols);
  std::transform( col.begin(), col.end()
                , width.begin()
                , cet::column_width
                );

  // prepare and emit the per-product information:
  for( unsigned row = 0, end = col[0].size(); row != end; ++row ) {
    std::string s;
    for( unsigned c = 0, end = ncols-1; c != end; ++c ) {
      s.append( cet::rpad(col[c][row], width[c], '.') )
       .append( " | ");
    }
    s.append( cet::lpad(col[ncols-1][row], width[ncols-1], '.') );
    std::cout << s << '\n';
  }

  std::cout << "\nTotal products (present, not present): "
            << present + not_present
            << " ("
            << present
            << ", "
            << not_present
            << ").\n\n";
}

// ======================================================================

DEFINE_ART_MODULE(FileDumperOutput)

// ======================================================================
