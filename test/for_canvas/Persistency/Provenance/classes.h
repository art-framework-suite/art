#include "RootClassMapping_t.h"

#include <string>

template
class TestProd<unsigned long, std::basic_string<char> >;

template
class TestProd<std::basic_string<char>, unsigned long>;
