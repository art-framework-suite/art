#ifndef JJJJSJDUDUAJ
#define JJJJSJDUDUAJ
// Define functions to allow reproduction of FPC errors.
// These functions must *not* be inlined or subject to LTO.
namespace arttest{
  double divit(double x, double y);
  double multit(double x, double y);
}
#endif
