#ifndef test_Integration_ToySource_h
#define test_Integration_ToySource_h

// Our simulated input file format is:
// A parameter in a parameter set, which contains a vector of vector of int.
// Each inner vector is a triplet of run/subrun/event number.
//   -1 means no new item of that type
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/IO/Sources/SourceHelper.h"
#include "art/Framework/Principal/fwd.h"

#include <string>
#include <vector>

namespace arttest {
  class ToySource;
}

class arttest::ToySource
{
public:
  ToySource(fhicl::ParameterSet const& ps,
          art::ProductRegistryHelper& help,
          art::SourceHelper const& sHelper);

  virtual ~ToySource() = default;

  void closeCurrentFile();

  virtual void readFile(std::string const &name,
                art::FileBlock*& fb) = 0;

  bool readNext(art::RunPrincipal* const& inR,
                art::SubRunPrincipal* const& inSR,
                art::RunPrincipal*& outR,
                art::SubRunPrincipal*& outSR,
                art::EventPrincipal*& outE);

protected:
  // Helper function to throw an exception with the appropriate text.
  static void throw_exception_from(const char* funcname);

  typedef std::vector<std::vector<int> > vv_t;
  typedef vv_t::const_iterator iter;

  iter    current_;
  iter    end_;
  fhicl::ParameterSet data_;
  vv_t fileData_;

  art::SourceHelper const & sHelper_;
  std::string currentFilename_;
  bool const throw_on_construction_;
  bool const throw_on_closeCurrentFile_;
  bool const throw_on_readNext_;
  bool const throw_on_readFile_;

  art::TypeLabel vtl_;
};
#endif /* test_Integration_ToySource_h */

// Local Variables:
// mode: c++
// End:
