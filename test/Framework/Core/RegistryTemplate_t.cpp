#include <cassert>
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/ParameterSet/Registry.h"

int main()
{
  edm::pset::Registry* psreg =
    edm::pset::Registry::instance();

  edm::ProcessHistoryRegistry* pnlreg =
    edm::ProcessHistoryRegistry::instance();

  assert( psreg );
  assert( pnlreg );
}
