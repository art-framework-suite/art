#include "art/Persistency/Common/CollectionUtilities.h"

#include <vector>
#include <concepts>
#include <catch2/catch_test_macros.hpp>

using namespace art;

namespace {
  struct two_in_container{
    template <typename T>
    void insert(T::iterator beg, T::iterator end){}
  };

  struct three_in_container{
    struct iterator {}
    iterator begin() {
      return iterator;
    }
    template<typename T>
    void insert(T::iterator self_end, T::iterator other_beg, T::iterator other_end){}}
  };

  template <typename T>
  concept can_two_arg_insert = requires (T& t1, T& t2) {
    { concatContainers(t1, t2) };
  }

  template <typename T>
  concept can_three_arg_insert = requires (T& t1, T& t2) {
    { concatContainers(t1, t2) };
  }
}



int main(){
  
}
