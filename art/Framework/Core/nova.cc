#include "novaapp.h"

#include <iostream>

int main( int argc, char* argv[] ) {
   int result = novaapp(argc,argv);
   std::cout
     << "Art has completed and will exit with status "
     << result
     << ".\n";
   return result;
}
