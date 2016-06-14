#ifndef art_Persistency_Provenance_ProductStatus_h
#define art_Persistency_Provenance_ProductStatus_h

/*----------------------------------------------------------------------

ProductStatus:

----------------------------------------------------------------------*/

namespace art {

  typedef unsigned char ProductStatus;

  namespace productstatus {
    inline ProductStatus present() {return 0x0;} // Product was made successfully
    inline ProductStatus neverCreated() {return 0x1;} // Product was not made successfully
    inline ProductStatus dropped() {return 0x2;} // Product was not made successfully
    inline ProductStatus unknown() {return 0xfe;} // Status unknown (used for backward compatibility)
    inline ProductStatus uninitialized() {return 0xff;} // Status not set

    inline bool present(ProductStatus status) {return status == present();}
    inline bool neverCreated(ProductStatus status) {return status == neverCreated();}
    inline bool dropped(ProductStatus status) {return status == dropped();}
    inline bool unknown(ProductStatus status) {return status == unknown();}
    inline bool uninitialized(ProductStatus status) {return status == uninitialized();}
  }  // productstatus

}  // art

#endif /* art_Persistency_Provenance_ProductStatus_h */

// Local Variables:
// mode: c++
// End:
