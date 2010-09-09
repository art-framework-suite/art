#ifndef FWCore_Catalog_SiteLocalConfig_h
#define FWCore_Catalog_SiteLocalConfig_h


#include <string>


namespace edm {
  class ActivityRegistry;
}

namespace edm {

  class SiteLocalConfig
  {
  public:
    SiteLocalConfig () {}
    virtual ~SiteLocalConfig() {}

    virtual const std::string dataCatalog (void) const = 0;
    virtual const std::string lookupCalibConnect (const std::string& input) const = 0;
    virtual const std::string rfioType (void) const = 0;

    // implicit copy constructor
    // implicit assignment operator
  private:
  };

}  // namespace edm

#endif  // FWCore_Catalog_SiteLocalConfig_h
