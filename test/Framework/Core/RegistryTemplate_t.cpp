#include <cassert>
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"

int main()
{
  art::ProcessHistoryRegistry* pnlreg =
    art::ProcessHistoryRegistry::instance();

  assert( pnlreg );
}
