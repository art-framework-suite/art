#include "art/Framework/IO/Root/setFileIndexPointer.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <memory>

namespace {
  void printFileIndex(std::string const& filename)
  {
    std::cout << std::string(30,'=') << '\n'
              << "File: " << filename.substr(filename.find_last_of('/')+1ul) << '\n';
    auto file = std::make_unique<TFile>(filename.data());
    if (!file || file->IsZombie())
      throw art::Exception(art::errors::FileOpenError)
        << "The file \"" << filename << "\" either does not exist\n"
        << "or an error was encountered when trying to open it.";

    auto metaDataTree = reinterpret_cast<TTree*>(file->Get(art::rootNames::metaDataTreeName().data()));

    auto fileIndexUniquePtr = std::make_unique<art::FileIndex>();
    auto findexPtr = &*fileIndexUniquePtr;
    art::setFileIndexPointer(file.get(), metaDataTree, findexPtr);

    std::cout << *findexPtr << std::endl;
  }
}

int main(int argc, char* argv[])
try
{
  if (argc < 2)
    throw art::Exception(art::errors::Configuration)
      << "Only \"" << argv[0] << "\" specified.\n"
      << "Must supply at least one filename.";

  std::vector<std::string> const filenames (argv+1, argv+argc);
  cet::for_all(filenames, [](auto const& name){ printFileIndex(name); });
}
catch(art::Exception const& e) {
  std::cerr << e.what() << '\n';
  return 1;
}
catch(...)
{
  std::cerr << "Some other exception has occurred.\n";
  return 2;
}
