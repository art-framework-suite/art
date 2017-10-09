// ------------------------------------------------------------
//
// ToyRawInput is a RawInputSource that pretends to reconstitute several
// products. It exercises the Source template.
//
// ------------------------------------------------------------

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/test/Integration/ToySource.h"
#include "cetlib/filepath_maker.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"

#include <memory>

namespace arttest {
  // ToyFile is the sort of class that experimenters who make use of
  // Source must write.
  class ToyFileReader;

  // ToyRawInput is an instantiation of the Source template.
  typedef art::Source<ToyFileReader> ToyRawFileInput;
} // namespace arttest

class arttest::ToyFileReader final : public ToySource {
public:
  ToyFileReader(fhicl::ParameterSet const& ps,
                art::ProductRegistryHelper& help,
                art::SourceHelper const& sHelper);

  void readFile(std::string const& name, art::FileBlock*& fb) override;
};

arttest::ToyFileReader::ToyFileReader(fhicl::ParameterSet const& ps,
                                      art::ProductRegistryHelper& help,
                                      art::SourceHelper const& sHelper)
  : ToySource(ps, help, sHelper)
{}

void
arttest::ToyFileReader::readFile(std::string const& name, art::FileBlock*& fb)
{
  if (throw_on_readFile_)
    throw_exception_from("readFile");
  fhicl::intermediate_table raw_config;
  cet::filepath_lookup_after1 lookupPolicy(".:");
  fhicl::parse_document(name, lookupPolicy, raw_config);
  fhicl::ParameterSet file_pset;
  make_ParameterSet(raw_config, file_pset);

  if (!file_pset.get_if_present("data", fileData_)) {
    throw art::Exception(art::errors::Configuration)
      << "ToyFileReader expects to find a parameter \"data\" representing a "
         "file's\n"
      << "contents in file " << name << "\n";
  }
  currentFilename_ = name;
  current_ = fileData_.begin();
  end_ = fileData_.end();
  fb = new art::FileBlock(art::FileFormatVersion(1, "ToyFileReader 2011a"),
                          currentFilename_);
}

DEFINE_ART_INPUT_SOURCE(arttest::ToyRawFileInput)
