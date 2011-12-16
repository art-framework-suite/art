#include "test/Utilities/openmpTestFunc.h"

#include "boost/thread/tss.hpp"

static boost::thread_specific_ptr<size_t> tsp;

__thread size_t kk = 0;

size_t openmpTestFunc(size_t numLoops, size_t mult) {   
   size_t total = 0;
#pragma omp for
   for (size_t i = 0; i<numLoops; ++i) {
      size_t j = i * mult;
      if (!tsp.get()) {
         tsp.reset(new size_t);
      }
      *tsp = j;
      kk += j;
#pragma omp critical(sec1)
      {
         std::cerr << (*tsp) << "\n";
         total += j;
      }
   }
   return total;
}
