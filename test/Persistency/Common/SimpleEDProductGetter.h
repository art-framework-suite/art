#ifndef DATAFORMAT_COMMON_TEST_SIMPLEEDPRODUCTGETTER_H
#define DATAFORMAT_COMMON_TEST_SIMPLEEDPRODUCTGETTER_H

#include <map>
#include <memory>

#include "boost/shared_ptr.hpp"

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Wrapper.h"


class SimpleEDProductGetter : public art::EDProductGetter
{
 public:

  typedef std::map<art::ProductID, boost::shared_ptr<art::EDProduct> > map_t;
  template <class T>
  void
  addProduct(art::ProductID const& id, std::auto_ptr<T> p)
  {
    typedef art::Wrapper<T> wrapper_t;

    boost::shared_ptr<wrapper_t> product(new wrapper_t(p));
    database[id] = product;
  }

  size_t size() const
  { return database.size(); }

  virtual art::EDProduct const* getIt(art::ProductID const& id) const
  {
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

#endif
