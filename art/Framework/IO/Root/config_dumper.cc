#include "cetlib/container_algorithms.h"

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "art/Persistency/Provenance/ParameterSetMap.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"

#include "boost/program_options.hpp"

#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Framework/IO/Root/rootNames.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace bpo = boost::program_options;

using std::back_inserter;
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;
using fhicl::ParameterSet;
using art::ParameterSetBlob;
using art::ParameterSetMap;

typedef vector<string> stringvec;

// Return true if the ParameterSet 'ps' was used to configure any
// module, regardless of its label.
inline bool is_module_configuration(ParameterSet const& ps)
{
  string label;
  // We could also check for "module_type", but that seems like needless
  // paranoia.
  return ps.get_if_present<string>("module_label", label);
}


// Print the human-readable form of a ParameterSet from which we strip
// the "module_label" parameter.
void print_stripped_module_pset(ParameterSet const& ps, ostream& output)
{
  string label = ps.get<string>("module_label");

  ParameterSet copy(ps);
  copy.erase("module_label");
  output << label << ":{\n";  
  output << copy.to_indented_string();
  output << "}\n\n";
}

// Print all the ParameterSets in the vector 'psets' which are module
// configurations.
void print_all_module_psets(vector<ParameterSet> const& psets,
                            ostream& output,
                            ostream& errors)
{
  for (size_t i = 0; i < psets.size(); ++i)
    {
      if (is_module_configuration(psets[i]))
        print_stripped_module_pset(psets[i], output);
    }  
}

// Return true if the ParameterSet 'ps' was used to configure a module
// with a label in module_labels.
inline bool matches_criteria(ParameterSet const& ps,
                      stringvec const& module_labels)
{
  string label;
  return ps.get_if_present<string>("module_label", label) && 
    cet::search_all(module_labels, label);
}

// Print all the ParameterSets in the vector 'psets' that are module
// configurations and for which the module label is included in the
// vector 'module_labels'.
void print_matching_parameter_sets(vector<ParameterSet> const& psets,
                                   stringvec const& module_labels,
                                   ostream& output,
                                   ostream& errors)
{
  for (size_t i = 0; i < psets.size(); ++i)
    {
      if (matches_criteria(psets[i], module_labels))
          print_stripped_module_pset(psets[i], output);
    }
}

// Read all the ParameterSets stored in 'file'. Write any error messages
// to errors.  Return false on failure, and true on success.
bool read_all_parameter_sets(TFile& file,
                             vector<ParameterSet>& all_read,
                             ostream& errors)
{
  ParameterSetMap psm;
  ParameterSetMap* psm_address = &psm;
  // Find the TTree that holds this data.
  TTree* metadata_tree = dynamic_cast<TTree*>(file.Get(art::rootNames::metaDataTreeName().c_str()));
  if (!metadata_tree)
    {
      errors << "Unable to find the metadata tree in file '" << file.GetName()
             << "';\nthis may not be an ART event data file.\n";
      return false;
    }

  art::setMetaDataBranchAddress(metadata_tree, psm_address);
  long bytes_read =  metadata_tree->GetEntry(0);
  if (bytes_read < 0)
    {
      errors << "Unable to read the metadata tree in file '" << file.GetName()
             << ";\nthis file appears to be corrupted.\n";
      return false;
    }

  for (ParameterSetMap::const_iterator
         i = psm.begin(),
         e = psm.end();
       i != e;
       ++i)
    {
      // Read the next ParameterSet directly into the output vector.
      all_read.push_back(ParameterSet());
      fhicl::make_ParameterSet(i->second.pset_, all_read.back());
    }
  return true;
}

// Extract all the requested module configuration ParameterSets (for
// modules with the given labels, run as part of processes of the given
// names) from the given TFIle. An empty list of process names means
// select all process names; an empty list of module labels means select
// all modules. The ParameterSets are written to the stream output, and
// error messages are written to the stream errors.
//
// Returns 0 to indicate success, and 1 on failure.
// Precondition: file.IsZombie() == false

// Caution: We pass 'file' by non-const reference because the TFile interface
// does not declare the functions we use to be const, even though they do not
// modify the underlying file.
int print_pset_from_file(TFile& file,
                         stringvec const& process_names,
                         stringvec const&  module_labels,
                         ostream& output,
                         ostream& errors)
{
  vector<ParameterSet> all_parameter_sets;
  if (! read_all_parameter_sets(file, all_parameter_sets, errors))
    {
      errors << "Unable to to read parameter sets.\n";
      return 1;
    }
  
  // Iterate through all the ParameterSets, printing the correct ones.
  // In this version, we do not yet use the process_names.

  if (module_labels.empty())
    print_all_module_psets(all_parameter_sets, output, errors);
  else
    print_matching_parameter_sets(all_parameter_sets, module_labels,
                                  output, errors);

  return 0;
};

// Extract all the requested module configuration ParameterSets (for
// modules with the given labels, run as part of processes of the given
// names) from the named files. An empty list of process names means
// select all process names; an empty list of module labels means select
// all modules. The ParameterSets are written to the stream output, and
// error messages are written to the stream errors.
//
// The return value is the number of files in which errors were
// encountered, and is thus 0 to indicate success.
int print_psets_from_files(stringvec const& file_names,
                           stringvec const& process_names,
                           stringvec const&  module_labels,
                           ostream& output,
                           ostream& errors)
{
  int rc = 0;
  for (stringvec::const_iterator
         i = file_names.begin(),
         e = file_names.end();
       i != e;
       ++i)
    {
      TFile current_file(i->c_str(), "READ");
      if (current_file.IsZombie())
        {
          ++rc;
          errors << "Unable to open file '" 
                 << *i
                 << "' for reading."
                 << "\nSkipping to next file.\n";
        }
      else
        {
          rc += print_pset_from_file(current_file,
                                     process_names,
                                     module_labels,
                                     output,
                                     errors);
        }
    }
  return rc;
}


int main(int argc, char* argv[])
{
  // ------------------
  // use the boost command line option processing library to help out
  // with command line options

  std::ostringstream descstr;

  descstr << argv[0]
          << " <options> [<source-file>]+";

  bpo::options_description desc(descstr.str());

  desc.add_options()
    ("help,h",                             "produce help message")
    ("label,l",   bpo::value<stringvec>(), "module label (multiple OK)")
    ("process,p", bpo::value<stringvec>(), "process name (Not yet implemented)")
    ("source,s",  bpo::value<stringvec>(), "source data file (multiple OK)");

  bpo::options_description all_opts("All Options");
  all_opts.add(desc);

  // Each non-option argument is interpreted as the name of a files to
  // be processed. Any number of filenames is allowed.
  bpo::positional_options_description pd;
  pd.add("source", -1);

  // The variables_map contains the actual program options.
  bpo::variables_map vm;
  try 
    {
      bpo::store(bpo::command_line_parser(argc,argv).options(all_opts).positional(pd).run(),
                 vm);
      bpo::notify(vm);
    }
  catch(bpo::error const& e)
    {
      std::cerr << "Exception from command line processing in " 
                << argv[0] << ": " << e.what() << "\n";
      return 2;
    }

  if (vm.count("help"))
    {
      std::cout << desc <<std::endl;
      return 1;
    }

  // Get the names of the processes we will look for; if none are
  // specified we want them all.
  stringvec process_names;

  // ------------------------------------------------------------
  // Handling of process names is not yet implemented.
  //   if (vm.count("process"))
  //     cet::copy_all(vm["process"].as<stringvec>(),
  //                   std::back_inserter(process_names));

  if (vm.count("process") > 0)
    {
      std::cerr << "Handling of process names is not yet implemented.\n";
      return 1;
    }
  //
  // ------------------------------------------------------------
  

  // Get the labels of the modules we will look for; if none are
  // specified we want them all.
  stringvec module_labels;
  if (vm.count("label"))
    cet::copy_all(vm["label"].as<stringvec>(),
                  std::back_inserter(module_labels));

  // Get the names of the files we will process.
  stringvec file_names;
  size_t file_count = vm.count("source");
  if (file_count < 1)
    {
      cerr << "One or more input files must be specified;"
           << " supply filenames as program arguments\n"
           << "For usage and options list, please do 'config_dumper --help'.\n";
      return 3;
    }
  file_names.reserve(file_count);
  cet::copy_all(vm["source"].as<stringvec>(),
                std::back_inserter(file_names));

  // Prepare for dealing with Root. We use the RootDictionaryManager to
  // load all necessary dictionaries.
  art::RootDictionaryManager dictionary_loader;

  return print_psets_from_files(file_names,
                                process_names,
                                module_labels,
                                cout,
                                cerr);
  
  // Testing.
  //   cout << "Specified module labels\n";
  //   cet::copy_all(module_labels,
  //                 std::ostream_iterator<string>(cout, ", "));
  //   cout << endl;

  //   cout << "Specified process names\n";
  //   cet::copy_all(process_names,
  //                 std::ostream_iterator<string>(cout, ", "));
  //   cout << endl;

  //   cout << "Specified input files\n";
  //   cet::copy_all(file_names,
  //                 std::ostream_iterator<string>(cout, ", "));
  //   cout << endl;
}
