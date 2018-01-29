// vim: set sw=2 expandtab :
#include "art/Utilities/CurrentProcessingContext.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

// namespace {
//
// art::ModuleDescription
// makeModuleDescription(std::string const& label)
//{
//  return art::ModuleDescription(fhicl::ParameterSet().id(), "", label, 1,
//  art::ProcessConfiguration());
//}
//
// static
// art::ModuleDescription
// moduleA = makeModuleDescription("aaa");
//
// static
// std::string
// pathName("path_a");
//
// static
// std::size_t
// pathNumber(21);
//
// static
// std::size_t
// slotInPath(13);
//
// static
// art::ModuleDescription const*
// p_moduleA = &moduleA;
//
// static
// std::string const*
// p_pathName = &pathName;
//
//} // unnamed namespace

// void
// test_default_ctor()
//{
//  art::CurrentProcessingContext ctx;
//  assert(ctx.moduleLabel() == 0);
//  assert(ctx.moduleDescription() == 0);
//  assert(ctx.slotInPath() == -1);
//  assert(ctx.pathInSchedule() == -1);
//}

// void
// test_activate()
//{
//  art::CurrentProcessingContext ctx(p_pathName, pathNumber, false);
//  ctx.activate(slotInPath, p_moduleA);
//  assert(ctx.moduleDescription() == p_moduleA);
//  assert(ctx.moduleLabel());
//  assert(*ctx.moduleLabel() == "aaa");
//  assert(ctx.slotInPath() == 13);
//  assert(ctx.pathInSchedule() == 21);
//}

// void
// test_deactivate()
//{
//  art::CurrentProcessingContext ctx(p_pathName, pathNumber, false);
//  ctx.activate(slotInPath, p_moduleA);
//  ctx.deactivate();
//  assert(ctx.moduleLabel() == 0);
//  assert(ctx.moduleDescription() == 0);
//}

void
work()
{
  // test_default_ctor();
  // test_deactivate();
}

int
main()
{
  // int rc = 0;
  // try {
  //  work();
  //}
  // catch (cet::exception& x) {
  //  std::cerr << "cet::exception caught\n";
  //  std::cerr << x.what() << '\n';
  //  rc = 1;
  //}
  // catch (std::exception& x) {
  //  std::cerr << "std::exception caught\n";
  //  std::cerr << x.what() << '\n';
  //  rc = 2;
  //}
  // catch (...) {
  //  std::cerr << "Unknown exception caught\n";
  //  rc = 3;
  //}
  // return rc;
}
