/*
 *
 *  CMSSW
 *
 */

#include "art/Persistency/Common/Wrapper.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

class CopyNoSwappy
{
 public:
  CopyNoSwappy() {}
  CopyNoSwappy(CopyNoSwappy const&) { /* std::cout << "copied\n"; */ }
  CopyNoSwappy& operator=(CopyNoSwappy const&) &  { /*std::cout << "assigned\n";*/ return *this;}
 private:
};

class SwappyNoCopy
{
 public:
  SwappyNoCopy() {}
  void swap(SwappyNoCopy&) { /* std::cout << "swapped\n";*/ }
 private:
  SwappyNoCopy(SwappyNoCopy const&); // not implemented
  SwappyNoCopy& operator=(SwappyNoCopy const&); // not implemented
};

void work()
{
  std::unique_ptr<CopyNoSwappy> thing(new CopyNoSwappy);
  art::Wrapper<CopyNoSwappy> wrap(thing);

  std::unique_ptr<SwappyNoCopy> thing2(new SwappyNoCopy);
  art::Wrapper<SwappyNoCopy> wrap2(thing2);


  std::unique_ptr<std::vector<double> >
    thing3(new std::vector<double>(10,2.2));
  assert(thing3->size() == 10);

  art::Wrapper<std::vector<double> > wrap3(thing3);
  assert(wrap3->size() == 10);
  assert(thing3.get() == 0);
}

int main()
{
  int rc = 0;
  try {
      work();
  }
  catch (...) {
      rc = 1;
      std::cerr << "Failure: unidentified exception caught\n";
  }
  return rc;
}
