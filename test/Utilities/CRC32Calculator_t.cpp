#include <cassert>
#include <iostream>
#include "art/Utilities/CRC32Calculator.h"

int main()
{
  art::CRC32Calculator crc32("type_label_instance_process");
  std::cout << "checksum = " << crc32.checksum() << std::endl;
  // This known result was calculated using python as a cross check
  unsigned int  knownResult = 1215348599;
  assert(crc32.checksum() == knownResult);
  art::CRC32Calculator emptyString_crc32("");
  assert(emptyString_crc32.checksum() == 0);
  std::cout << "Empty string checksum = "
            << emptyString_crc32.checksum() << std::endl;
}
