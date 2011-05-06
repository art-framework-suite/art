
#include <iostream>


#include <cppunit/extensions/HelperMacros.h>

// ----------------------------------------------
class testmaker: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testmaker);
CPPUNIT_TEST(makerTest);
CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}
  void makerTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testmaker);

void testmaker::makerTest()
//int main()
{
  std::string param1 =
    "string module_type = \"TestMod\"\n "
    " string module_label = \"t1\"";

  std::string param2 =
    "string module_type = \"TestMod\" "
    "string module_label = \"t2\"";

  /*try {

    artplugin::PluginManager::configure(artplugin::standard::config());
    Factory* f = Factory::get();

    //Factory::Iterator ib(f->begin()),ie(f->end());
    //for(;ib!=ie;++ib)
    //  {
    //std::cout << (*ib)->name() << std::endl;
    // }

    std::shared_ptr<ParameterSet> p1 = makePSet(*art::pset::parse(param1.c_str()));;
    std::shared_ptr<ParameterSet> p2 = makePSet(*art::pset::parse(param2.c_str()));;

    std::cerr << p1->get<std::string>("module_type");

    art::ActionTable table;

    art::ProductRegistry preg;

    std::auto_ptr<Worker> w1 = f->makeWorker(*p1, preg, table, "PROD", 0, 0);
    std::auto_ptr<Worker> w2 = f->makeWorker(*p2, preg, table, "PROD", 0, 0);
  }
  catch(cet::exception& e) {
      std::cerr << "cet::exception: " << e.explain_self() << std::endl;
      throw;
  }
  catch(std::exception& e) {
      std::cerr << "std::Exception: " << e.what() << std::endl;
      throw;
  }
  catch(...) {
      std::cerr << "weird exception" << std::endl;
      throw;
  }

  return 0;*/
}
