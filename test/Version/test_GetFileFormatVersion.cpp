#include "art/Version/GetFileFormatVersion.h"
#include <cassert>
#include <iostream>
int main(int argc, char* argv[])
{
  assert(edm::getFileFormatVersion() == 11);
  std::cout << "Running " << argv[0] << std::endl;
}
