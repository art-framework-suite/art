#ifndef test_Persistency_Common_SimpleEDProductGetter_h
#define test_Persistency_Common_SimpleEDProductGetter_h

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Wrapper.h"
#include "cpp0x/memory"
#include <map>


class SimpleEDProductGetter : public art::EDProductGetter {
public:

  typedef std::map<art::ProductID, std::shared_ptr<art::EDProduct> > map_t;
  template <class T>
  void
  addProduct(art::ProductID const & id, std::auto_ptr<T> p) {
    typedef art::Wrapper<T> wrapper_t;
    std::shared_ptr<wrapper_t> product(new wrapper_t(p));
    database[id] = product;
  }

  size_t size() const
  { return database.size(); }

  virtual art::EDProduct const * getIt(art::ProductID const & id) const {
    map_t::const_iterator i = database.find(id);
    if (i == database.end())
      throw art::Exception(art::errors::ProductNotFound, "InvalidID")
          << "No product with ProductID "
          << id
          << " is available from this EDProductGetter\n";
    return i->second.get();
  }

private:
  map_t database;
};

#endif /* test_Persistency_Common_SimpleEDProductGetter_h */

// Local Variables:
// mode: c++
// End:
