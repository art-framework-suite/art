#include "art/Framework/IO/Root/GetFileFormatVersion.h"
#include <cassert>
#include <iostream>
int main(int /*argc*/, char* argv[])
{
  assert(art::getFileFormatVersion() == 8);
  std::cout << "Running " << argv[0] << std::endl;
}
