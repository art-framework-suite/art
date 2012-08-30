#include "art/Utilities/ensureTable.h"

#include "fhiclcpp/intermediate_table.h"

void
art::ensureTable(fhicl::intermediate_table & table,
                 std::string const  & name) {
  if (!table.exists(name)) {
    table.putEmptyTable(name);
  }
}
