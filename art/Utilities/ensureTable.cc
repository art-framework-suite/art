#include "art/Utilities/ensureTable.h"

#include "cetlib/exception.h"
#include "fhiclcpp/intermediate_table.h"

using namespace fhicl;

void
art::ensureTable(intermediate_table & table,
                 std::string const  & name)
{
  extended_value & val = table[name];
  if (val.is_a(NIL))
  { val = extended_value(false, TABLE, extended_value::table_t()); }
  else if (! val.is_a(TABLE))
    throw cet::exception("BAD_CONFIG")
        << "Configuration item \""
        << name
        << "\" exists but is not a table.\n";
}
