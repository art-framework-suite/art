#ifndef art_Framework_IO_ProductMerge_MergeOpBase_h
#define art_Framework_IO_ProductMerge_MergeOpBase_h

#include <cstddef>

namespace art {
  class MergeOpBase;

  class EDProduct;
  class Event;
  class InputTag;
  class PtrRemapper;
  class TypeID;
}

class art::MergeOpBase {
public:
  virtual
  InputTag const &inputTag() const = 0;

  virtual
  TypeID const &inputType() const = 0;

  virtual
  std::string const &outputInstanceLabel() const = 0;

  virtual
  void
  mergeAndPut(Event &e, PtrRemapper const &remap) const = 0;

  virtual
  void
  initProductList(size_t nSecondaries) = 0;

};
#endif /* art_Framework_IO_ProductMerge_MergeOpBase_h */

// Local Variables:
// mode: c++
// End:
