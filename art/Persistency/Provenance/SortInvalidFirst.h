#ifndef art_Persistency_Provenance_SortInvalidFirst_h
#define art_Persistency_Provenance_SortInvalidFirst_h

#include <functional>

namespace art {
   template <typename T> class SortInvalidFirst;
}

template <typename T> class art::SortInvalidFirst :
   public std::binary_function<T, T, bool> {
 public:
   SortInvalidFirst() : invalidValue_() {}
   explicit SortInvalidFirst(T const & invalidValue)
      :
      invalidValue_(invalidValue)
   {}
   bool operator()(T const &left, T const &right) const {
      if (left == invalidValue_ && right != invalidValue_) {
         return true;
      } else if (right == invalidValue_ && left != invalidValue_) {
         return false;
      } else {
         return left < right;
      }
   }
private:
   T invalidValue_;
};

#endif /* art_Persistency_Provenance_SortInvalidFirst_h */

// Local Variables:
// mode: c++
// End:
