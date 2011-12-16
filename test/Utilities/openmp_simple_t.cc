#include <iostream>
#include "test/Utilities/openmpTestFunc.h"

int main(int, char **)
{
  size_t total = openmpTestFunc(10, 20);
  std::cout << "Total: " << total << std::endl;
}
