#include "art/Framework/Core/CurrentProcessingContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exception.h"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

namespace
{
  // Forward declare test helpers
  art::ModuleDescription makeModuleDescription(std::string const& label);
  void setup_ctx(art::CurrentProcessingContext& ctx);

  // Icky global junk, to mock lifetime of ModuleDescription.
  static art::ModuleDescription moduleA = makeModuleDescription("aaa");
  static std::string pathName("path_a");
  static std::size_t pathNumber(21);
  static std::size_t slotInPath(13);

  static art::ModuleDescription const* p_moduleA = &moduleA;
  static std::string const*            p_pathName = &pathName;

  // Test helpers
  art::ModuleDescription makeModuleDescription(std::string const& label)
  {
    art::ModuleDescription temp;
    temp.moduleLabel_ = label;
    return temp;
  }

  void setup_ctx(art::CurrentProcessingContext& ctx)
  {
    assert(p_moduleA);
    art::CurrentProcessingContext temp(p_pathName, pathNumber, false);
    temp.activate(slotInPath, p_moduleA);
    ctx = temp;
  }

} // namespace


void test_default_ctor()
{
  art::CurrentProcessingContext ctx;
  assert(ctx.moduleLabel() == 0);
  assert(ctx.moduleDescription() == 0);
  assert(ctx.slotInPath() == -1);
  assert(ctx.pathInSchedule() == -1);
}

void test_activate()
{
  art::CurrentProcessingContext ctx(p_pathName, pathNumber, false);
  ctx.activate(slotInPath, p_moduleA);
  {
    art::CurrentProcessingContext const& r_ctx = ctx;
    assert(r_ctx.moduleDescription() == p_moduleA);
    assert(r_ctx.moduleLabel());
    assert(*r_ctx.moduleLabel() == "aaa");
    assert(r_ctx.slotInPath() == 13);
    assert(r_ctx.pathInSchedule() == 21);
  }
}

void test_deactivate()
{
  art::CurrentProcessingContext ctx;
  setup_ctx(ctx);
  ctx.deactivate();
  assert(ctx.moduleLabel() == 0);
  assert(ctx.moduleDescription() == 0);
}


int work()
{
  test_default_ctor();
  test_deactivate();
  return 0;
}

int main()
{
  int rc = -1;
  try { rc = work(); }
  catch (cet::exception& x) {
      std::cerr << "cet::exception caught\n";
      std::cerr << x.what() << '\n';
      rc = -2;
  }
  catch (std::exception& x) {
      std::cerr << "std::exception caught\n";
      std::cerr << x.what() << '\n';
      rc = -3;
  }
  catch (...) {
      std::cerr << "Unknown exception caught\n";
      rc = -4;
  }
  return rc;
}
