#include "art/Framework/IO/Root/rootNames.h"
#include "art/Framework/IO/Root/setFileIndexPointer.h"
#include "art/Utilities/Exception.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[])
try
{
  if (argc != 2)
    throw art::Exception(art::errors::Configuration);

  auto file = std::make_unique<TFile>(argv[1]);
  if (!file || file->IsZombie()) return 1;

  auto metaDataTree = reinterpret_cast<TTree*>(file->Get(art::rootNames::metaDataTreeName().c_str()));

  auto fileIndexUniquePtr = std::make_unique<art::FileIndex>();
  auto findexPtr = &*fileIndexUniquePtr;
  art::setFileIndexPointer(file.get(), metaDataTree, findexPtr);

  std::cout << *findexPtr << std::endl;
}
catch(...)
{
  return 2;
}
