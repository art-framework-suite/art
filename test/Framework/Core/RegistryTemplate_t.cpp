#include <cassert>
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/ParameterSet/Registry.h"

int main()
{
  art::pset::Registry* psreg =
    art::pset::Registry::instance();

  art::ProcessHistoryRegistry* pnlreg =
    art::ProcessHistoryRegistry::instance();

  assert( psreg );
  assert( pnlreg );
}
