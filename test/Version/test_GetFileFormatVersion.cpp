#include "art/Version/GetFileFormatVersion.h"
#include <cassert>
#include <iostream>
int main(int argc, char* argv[])
{
  assert(art::getFileFormatVersion() == 1);
  std::cout << "Running " << argv[0] << std::endl;
}
