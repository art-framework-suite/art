// DataFlowDumper
//
// DataFlowDumper is an output module that creates a printout of the
// "data flow" that produced an Event. This means it writes
// information naming each data product in the event, what module that
// product was created by, and what data products were read by that
// module (the 'parents' of the original data product).
//

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/ProvenanceDumper.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace art {
  // DataFlow is the class we define to provide our specialization of
  // the ProvenanceDumper class template, to create the DataFlowDumper
  // class.
  class DataFlow;
  using DataFlowDumper = ProvenanceDumper<DataFlow>;
}

class art::DataFlow {
public:
  struct Config {
    fhicl::Atom<std::string> dotfile { fhicl::Name("dotfile"), "flow.dot" };
    fhicl::Atom<std::string> colorscheme { fhicl::Name("colorscheme"), "set312" };
    fhicl::Atom<int> debuglevel { fhicl::Name("debuglevel"), 0 };
  };

  explicit DataFlow(fhicl::TableFragment<Config> const& cfg);

  // Prepare to write out the graph for an event.
  void preProcessEvent();

  // Finalize writing the graph for an event.
  void postProcessEvent();

  // Write the graph description lines for the data product associated
  // with the given provenance.
  void processEventProvenance(art::Provenance const& prov);

private:
  std::ofstream out_;
  int nEvents_;
  std::string colorscheme_;
  int debug_;
};

//-----------------------------------------------------------------

art::DataFlow::DataFlow(fhicl::TableFragment<Config> const& cfg) :
  out_(cfg().dotfile()),
  nEvents_(0),
  colorscheme_(cfg().colorscheme()),
  debug_(cfg().debuglevel())
{
  if (!out_) {
    throw art::Exception(art::errors::FileOpenError) << 
      "Failed to create output file: " << cfg().dotfile();
  }
}

void art::DataFlow::preProcessEvent() {
  out_ << "digraph d" << nEvents_ << " {\n";
}

void art::DataFlow::postProcessEvent() {
  out_ << "}\n\n";
  ++nEvents_;
}

// Write the identifier for the node for the product to which this
// Provenance belongs.
void write_id(art::BranchID const& bid, std::ostream& os) {
  os << "\"b"
     << bid
     << '\"';
}

void write_id(art::Provenance const& p, std::ostream& os) {
  write_id(p.branchID(), os);
}

// format_product_node defines the format for the product nade.
void format_product_node(std::string const& fcn,
         std::string const& pin,
         std::ostream& os)
{
  os << " [label = \"" << fcn;
  if (!pin.empty()) os << "/" << pin;
  os << "\" shape = box];\n";
}

// Write the line defining the node for the product to which this
// Provenance belongs.
void write_product_node(art::Provenance const& p,
                        std::ostream& os,
                        int debug) {
  if (debug > 0) {
    os << "# write_product_node for provenance: " << &p << '\n';
  }
  write_id(p,os);
  format_product_node(p.friendlyClassName(), p.productInstanceName(), os);
}

void write_product_node(art::BranchID const& bid,
                        std::ostream& os,
                        int debug) {
  if (debug > 0) {
    os << "# write_product_node for bid: " << bid << '\n';
  }
  // Access to the productList is cheap, so not really worth caching.
  auto const& pmd = art::ProductMetaData::instance();
  auto const& plist = pmd.productList(); // note this is a map
  // The mapped_type in the map contains all the information we want,
  // but we have to do a linear search through the map to find the one
  // with the right BranchID.
  auto it = std::find_if(begin(plist), end(plist),
                         [&bid](auto const& keyval) {
                           return keyval.second.branchID() == bid;
                         });
  if (it == plist.end()) {
    os << "#Missing information for branch with id " << bid << '\n';
    return;
  }
  write_id(bid, os);
  format_product_node(it->second.friendlyClassName(),
                      it->second.productInstanceName(),
                      os);
}

void write_module_id(art::Provenance const& p, std::ostream& os) {
  os << '\"'
     << p.moduleLabel()
     << '/'
     << p.processName()
     << '\"';
}

std::size_t color(std::string const& procname) {
  static std::vector<std::string> names_seen;
  auto it = std::find(begin(names_seen), end(names_seen), procname);
  if (it == end(names_seen)) {
    names_seen.push_back(procname);
    return names_seen.size();
  }
  return std::distance(begin(names_seen), it)+1;
}

void write_module_node(art::Provenance const& p,
                        std::string const& colorscheme,
                        std::ostream& os) {
  os << " [ colorscheme="
     << colorscheme
     << " color="
     << color(p.processName())
     << " style=filled ];\n";
}

void write_creator_line(art::Provenance const& p,
                        std::string const& colorscheme,
                        std::ostream& os,
                        int debug) {
  if (debug > 0) {
    os << "# write_creator_line for provenance: " << &p << '\n';
  }
  write_module_id(p, os);
  write_module_node(p, colorscheme, os);
  write_module_id(p, os);
  os << " -> ";
  write_id(p, os);
  os << ";\n";
}

void write_parent_id(art::BranchID const& parent,
                     std::ostream& os) {
 os << 'b' << parent;
}

void write_parentage_line(art::Provenance const& p,
                          art::BranchID const& parent,
                          std::ostream& os,
                          int debug) {
  if (debug > 0) {
    os << "# write_parentage_line for provenance: " << &p
       << " parent " << parent << '\n';
  }  
  write_parent_id(parent, os);
  os << " -> ";
  write_module_id(p, os);
  os << ";\n";
}

void art::DataFlow::processEventProvenance(art::Provenance const& p) {
  write_product_node(p, out_, debug_);
  write_creator_line(p, colorscheme_, out_, debug_);
  for (art::BranchID const& parent : p.parents()) {
    write_parentage_line(p, parent, out_, debug_);
  }
}

DEFINE_ART_MODULE(art::DataFlowDumper)
