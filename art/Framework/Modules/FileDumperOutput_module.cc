// ======================================================================
//
// FileDumperOutput.cc: "dump contents of a file"
//
// Proposed output format (Feature #941):
// Process Name | Module Label | Process Label | Data Product type | size
//
// ======================================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/ConfigTable.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/column_width.h"
#include "cetlib/lpad.h"
#include "cetlib/rpad.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace art {
  class FileDumperOutput;
}

using fhicl::ParameterSet;

class art::FileDumperOutput : public OutputModule {
public:

  struct Config {
    fhicl::TableFragment<OutputModule::Config> omConfig;
    fhicl::Atom<bool> onDemandProduction           { fhicl::Name("onDemandProduction")      , false };
    fhicl::Atom<bool> wantProductFullClassName     { fhicl::Name("wantProductFullClassName"), true };
    fhicl::Atom<bool> wantProductFriendlyClassName { fhicl::Name("wantProductFriendlyClassName"), wantProductFullClassName() };
    fhicl::Atom<bool> resolveProducts { fhicl::Name("resolveProducts"), true };
    fhicl::Atom<bool> onlyIfPresent   { fhicl::Name("onlyIfPresent"), false };
  };

  using Parameters = ConfigTable<Config, OutputModule::Config::KeysToIgnore>;

  explicit FileDumperOutput(Parameters const &);


private:
  void write(EventPrincipal & e) override;
  void writeRun(RunPrincipal & r) override;
  void writeSubRun(SubRunPrincipal & sr) override;
  void readResults(ResultsPrincipal const & resp) override;

  template <typename P>
  void printPrincipal(P const & p);

  bool wantOnDemandProduction_;
  bool wantProductFullClassName_;
  bool wantProductFriendlyClassName_;
  bool wantResolveProducts_;
  bool wantPresentOnly_;
};  // FileDumperOutput

art::FileDumperOutput::
FileDumperOutput(art::FileDumperOutput::Parameters const & ps)
  :
  OutputModule{ps().omConfig, ps.get_PSet()},
  wantOnDemandProduction_{ps().onDemandProduction()},
  wantProductFullClassName_{ps().wantProductFullClassName()},
  wantProductFriendlyClassName_{ps().wantProductFriendlyClassName()},
  wantResolveProducts_{ps().resolveProducts()},
  wantPresentOnly_{ps().onlyIfPresent()}
{
}

void
art::FileDumperOutput::
write(EventPrincipal & e)
{
  printPrincipal(e);
}

void
art::FileDumperOutput::
writeRun(RunPrincipal & r)
{
  printPrincipal(r);
}

void
art::FileDumperOutput::
writeSubRun(SubRunPrincipal & sr)
{
  printPrincipal(sr);
}

void
art::FileDumperOutput::
readResults(ResultsPrincipal const & resp)
{
  printPrincipal(resp);
}

template <typename P>
void
art::FileDumperOutput::
printPrincipal(P const & p)
{
  if (!p.size()) { return; } // Nothing to do.
  std::cout << "PRINCIPAL TYPE: " << BranchTypeToString(p.branchType()) << std::endl;
  // prepare the data structure, a sequence of columns:
  using column = std::vector<std::string>;
  constexpr unsigned int ncols {6};
  std::vector<column> col(ncols);
  // provide column headings:
  col[0].push_back("PROCESS NAME");
  col[1].push_back("MODULE LABEL");
  col[2].push_back("PRODUCT INSTANCE NAME");
  col[3].push_back("DATA PRODUCT TYPE");
  col[4].push_back("PRODUCT FRIENDLY TYPE");
  col[5].push_back("SIZE");
  size_t present {0};
  size_t not_present {0};
  // insert the per-product data:
  for (auto const& pr : p) {
    auto const& g = *pr.second;
    auto const& oh = p.getForOutput(g.productDescription().branchID(), wantResolveProducts_);

    EDProduct const* product = oh.isValid() ? oh.wrapper() : nullptr;
    bool const productPresent = product != nullptr && product->isPresent();

    if (productPresent) {
      ++present;
    }
    else {
      ++not_present;
    }

    if ((!wantPresentOnly_) || productPresent) {
      col[0].push_back(g.processName());
      col[1].push_back(g.moduleLabel());
      col[2].push_back(g.productInstanceName());
      col[3].push_back(g.productDescription().producedClassName());
      col[4].push_back(g.productDescription().friendlyClassName());
      if (productPresent) {
        col[5].push_back(product->productSize());
      }
      else {
        col[5].push_back(g.onDemand() ? "o/d" : "?");
      }
    }
  }
  // determine each column's width:
  std::vector<unsigned> width(ncols);
  std::transform(col.begin(),
                 col.end(),
                 width.begin(),
                 &cet::column_width);
  // prepare and emit the per-product information:
  for (unsigned row = 0, end = col[0].size(); row != end; ++row) {
    std::string s;
    for (unsigned c = 0, end = ncols - 1; c != end; ++c) {
      if (c == 3 && ! wantProductFullClassName_) { continue; }
      if (c == 4 && ! wantProductFriendlyClassName_) { continue; }
      s.append(cet::rpad(col[c][row], width[c], '.'))
      .append(" | ");
    }
    s.append(cet::lpad(col[ncols - 1][row], width[ncols - 1], '.'));
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

DEFINE_ART_MODULE(art::FileDumperOutput)
