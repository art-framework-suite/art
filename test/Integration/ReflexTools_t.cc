// Test of the ReflexTools functions.

#define BOOST_TEST_MODULE (ReflexTools_t)
#include "boost/test/auto_unit_test.hpp"

#include <iostream>
#include <typeinfo>
#include <vector>

#include "Reflex/Type.h"

#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "art/Persistency/Common/Wrapper.h"

using Reflex::Type;
using Reflex::TypeTemplate;

struct TestFixture {
  TestFixture() {
    static art::RootDictionaryManager rdm_s;
  }
};

BOOST_FIXTURE_TEST_SUITE(ReflexTools_t, TestFixture)

BOOST_AUTO_TEST_CASE(default_is_invalid)
{
  Type t;
  BOOST_REQUIRE(!t);
}

BOOST_AUTO_TEST_CASE(no_dictionary_is_invalid)
{
  Type t(Type::ByName("ThereIsNoTypeWithThisName"));
  BOOST_REQUIRE(!t);
}

BOOST_AUTO_TEST_CASE(find_nested)
{
  Type intvec(Type::ByName("std::vector<int>"));
  BOOST_REQUIRE(intvec);

  Type found_type;

  BOOST_REQUIRE(art::find_nested_type_named("const_iterator",
                                             intvec,
                                             found_type));

  BOOST_REQUIRE(!art::find_nested_type_named("WankelRotaryEngine",
                                              intvec,
                                              found_type));
}

BOOST_AUTO_TEST_CASE(burrowing)
{
  Type wrapper_type(Type::ByTypeInfo(typeid(std::vector<int>)));
  BOOST_REQUIRE(wrapper_type);
  Type v_type;
  BOOST_REQUIRE(art::find_nested_type_named("value_type",
                                             wrapper_type,
                                             v_type));
  BOOST_REQUIRE(v_type);
  BOOST_REQUIRE(!v_type.IsTypedef());
  BOOST_REQUIRE(v_type.IsFundamental());
  BOOST_REQUIRE(v_type == Type::ByName("int"));
  BOOST_REQUIRE(v_type.TypeInfo() == typeid(int));
}

BOOST_AUTO_TEST_CASE(burrowing_failure)
{
  Type not_a_vector(Type::ByName("double"));
  BOOST_REQUIRE(not_a_vector);
  Type no_such_v_type;
  BOOST_REQUIRE(!no_such_v_type);
  BOOST_REQUIRE(!art::find_nested_type_named("value_type",
                                              not_a_vector,
                                              no_such_v_type));
  BOOST_REQUIRE(!no_such_v_type);
}

BOOST_AUTO_TEST_CASE(primary_template_id)
{
  Type intvec(Type::ByName("std::vector<int>"));
  TypeTemplate vec(intvec.TemplateFamily());

  // The template std::vector has two template parameters, thus the
  // '2' in the following line.
  TypeTemplate standard_vec(TypeTemplate::ByName("std::vector",2));
  BOOST_REQUIRE(!standard_vec);
  BOOST_REQUIRE(vec != standard_vec);

  // Reflex in use by CMS as of 26 Feb 2007 understands vector to have
  // one template parameter; this is not standard.
  TypeTemplate nonstandard_vec(TypeTemplate::ByName("std::vector",1));
  BOOST_REQUIRE(nonstandard_vec);
  BOOST_REQUIRE(vec == nonstandard_vec);
}

BOOST_AUTO_TEST_CASE(not_a_template_instance)
{
  Type not_a_template(Type::ByName("double"));
  BOOST_REQUIRE(not_a_template);
  TypeTemplate nonesuch(not_a_template.TemplateFamily());
  BOOST_REQUIRE(!nonesuch);
}

BOOST_AUTO_TEST_CASE(cint_wrapper_name)
{
  BOOST_REQUIRE_EQUAL(art::cint_wrapper_name("std::string"),
                      std::string("art::Wrapper<string>"));
}

BOOST_AUTO_TEST_CASE(type_of_template_arg)
{
  Reflex::Type wrapper(Reflex::Type::ByName("art::Wrapper<int>"));
  BOOST_REQUIRE(wrapper);
  Reflex::Type arg(art::type_of_template_arg(wrapper, 0));
  BOOST_REQUIRE(arg);
  BOOST_REQUIRE_EQUAL(arg.Name(Reflex::FINAL), std::string("int"));
}

BOOST_AUTO_TEST_CASE(is_instantiation_of)
{
  Reflex::Type wrapper(Reflex::Type::ByName("art::Wrapper<int>"));
  BOOST_REQUIRE(wrapper);
  BOOST_REQUIRE(art::is_instantiation_of(wrapper, "art::Wrapper"));
}

BOOST_AUTO_TEST_SUITE_END()
