// ------------------------------------------------------------
//
// ToyRawInput is a RawInputSource that pretends to reconstitute several
// products. It exercises the Source template.
//
// ------------------------------------------------------------

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/SourceTraits.h"
#include "art/Framework/IO/Sources/Source.h"
#include "art/test/Integration/ToySource.h"

namespace arttest {
  // ToyFile is the sort of class that experimenters who make use of
  // Source must write.
  class ToyReader;
}

namespace art {
  // We don't want the file services: we must say so by specializing the
  // template *before* specifying the typedef.
  template<>
  struct Source_wantFileServices<arttest::ToyReader> {
    static constexpr bool value = false;
  };
}

namespace arttest {
  // ToyRawInput is an instantiation of the Source template.
  typedef art::Source<ToyReader> ToyRawInput;
}

class arttest::ToyReader final : public ToySource {
public:
  ToyReader(fhicl::ParameterSet const& ps,
            art::ProductRegistryHelper& help,
            art::SourceHelper const& sHelper);

  void readFile(std::string const &name,
                art::FileBlock*& fb) override;
};


arttest::ToyReader::ToyReader(fhicl::ParameterSet const& ps,
                              art::ProductRegistryHelper& help,
                              art::SourceHelper const& sHelper)
  :
  ToySource(ps, help, sHelper)
{
}

void
arttest::ToyReader::readFile(std::string const &name,
                             art::FileBlock*& fb)
{
  if (throw_on_readFile_) throw_exception_from("readFile");
  if (!data_.get_if_present(name, fileData_))
  {
    throw art::Exception(art::errors::Configuration)
      << "ToyReader expects to find a parameter representing a file's\n"
      << "contents whose name is "
      << name
      << "\n";
  }
  currentFilename_ = name;
  current_ = fileData_.begin();
  end_ = fileData_.end();
  fb = new art::FileBlock(art::FileFormatVersion(1, "ToyReader 2011a"),
                          currentFilename_);
}


DEFINE_ART_INPUT_SOURCE(arttest::ToyRawInput)
