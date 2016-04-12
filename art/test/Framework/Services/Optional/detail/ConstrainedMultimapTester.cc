#include "art/Framework/Services/Optional/detail/constrained_multimap.h"

#include <iostream>
#include <random>

using namespace art::detail;

//==========================================================
namespace {

  template <typename MAP>
  void print ( MAP const & map, std::string const & type ) {
    std::cout << " Values for map: " << type << std::endl;
    unsigned i = map.size();
    for ( auto const & entry : map ) {
      std::cout << " ["<<i--<<"] "<< entry.first << " " << entry.second << std::endl;
    }
    std::cout << std::endl;
  }

}

//==========================================================
int main() {

  std::default_random_engine engine;
  std::uniform_int_distribution<int>     dist_int (1,100 );
  std::uniform_real_distribution<double> dist_real(0,100.);

  constrained_multimap<8u,int,std::string>     int_map;
  constrained_multimap<9u,double,std::string>  double_map1;
  constrained_multimap<16u,double,std::string> double_map2;

  for ( unsigned i(0) ; i != 100 ; ++i ) {

    const int int_val = dist_int( engine );
    int_map.maybe_emplace( int_val, "fun stuff" );

    const double real_val = dist_real( engine );
    double_map1.maybe_emplace( real_val , "other stuff");
    double_map2.maybe_emplace( real_val , "other stuff");

  }

  print( int_map    , "int_map"     );
  print( double_map1, "double_map1" );
  print( double_map2, "double_map2" );

}
