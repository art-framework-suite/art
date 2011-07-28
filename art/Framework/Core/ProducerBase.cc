/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/ProducerBase.h"

#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Framework/Core/TypeLabelList.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include <sstream>

namespace art
{

  ProducerBase::ProducerBase() :
    ProductRegistryHelper()
  { }

  ProducerBase::~ProducerBase()
  { }

  void
  ProducerBase::registerProducts(std::shared_ptr<ProducerBase> producer,
                                 MasterProductRegistry* preg,
                                 ModuleDescription const& md)
  {
    if (typeLabelList().empty() ) return;
    TypeLabelList& plist = typeLabelList();
    ProductRegistryHelper::addToRegistry(plist.begin(),
                                         plist.end(), md,
                                         *preg, false);
  }

}  // art
