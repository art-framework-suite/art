
#include "art/Utilities/MallocOpts.h"
#include <cassert>
#include <iostream>
using namespace std;

int
main()
{
  art::MallocOptionSetter& mo = art::getGlobalOptionSetter();
  art::MallocOpts mycopy = mo.get(), defaultt;

  assert(mo.retrieveFromEnv() == false);
  assert(mo.hasErrors() == false);
  mo.set_mmap_max(1);
  mo.adjustMallocParams();
  assert(mo.hasErrors() == false);

#if defined(__x86_64__) || defined(__i386__)
  assert(mo.retrieveFromCpuType() == true);
  assert(mo.get() == mycopy);
  assert(defaultt != mycopy);
#endif

  return 0;
}
