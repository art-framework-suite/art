#ifndef art_Persistency_Provenance_ProductID_h
#define art_Persistency_Provenance_ProductID_h

/*----------------------------------------------------------------------

ProductID: A unique identifier for each EDProduct within a process.
Used only in Ptr and similar classes.

The high order 16 bits is the process index, identifying the process
in which the product was created.  Exception: An index of 0 means that
the product was created prior to the new format (i.e. prior to CMSSW_3_0_0.

The low order 16 bits is the product index, identifying the product
in which the product was created.  An index of zero means no product.

----------------------------------------------------------------------*/

#include <iosfwd>

namespace art {

  typedef unsigned short ProcessIndex;
  typedef unsigned short ProductIndex;

  class ProductID
  {
  public:
    ProductID()
    : processIndex_(0)
    , productIndex_(0)
    { }

    ProductID(ProcessIndex processIndex, ProductIndex productIndex)
    : processIndex_(processIndex)
    , productIndex_(productIndex)
    { }

    bool isValid() const {return productIndex_ != 0;}

    ProcessIndex processIndex() const {return processIndex_;}
    ProcessIndex productIndex() const {return productIndex_;}

  private:
    ProcessIndex processIndex_;
    ProductIndex productIndex_;
  };

  inline
  bool operator==(ProductID const& lh, ProductID const& rh) {
    return lh.processIndex() == rh.processIndex() && lh.productIndex() == rh.productIndex();
  }
  inline
  bool operator!=(ProductID const& lh, ProductID const& rh) {
    return !(lh == rh);
  }

  bool operator<(ProductID const& lh, ProductID const& rh);

  std::ostream&
  operator<<(std::ostream& os, ProductID const& id);
}
#endif /* art_Persistency_Provenance_ProductID_h */

// Local Variables:
// mode: c++
// End:
