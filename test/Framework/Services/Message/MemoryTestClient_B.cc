#include "test/Framework/Services/Message/MemoryTestClient_B.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <iomanip>
#include <string>

namespace arttest
{

int  MemoryTestClient_B::nevent = 0;

MemoryTestClient_B::MemoryTestClient_B( fhicl::ParameterSet const & ps)
  : vsize(0)
{
  int pattern = ps.get<int>("pattern",1);
  mf::LogWarning("memoryPattern") << "Memory Pattern selected: " << pattern;
  initializeMemoryPattern(pattern);
}

void
  MemoryTestClient_B::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  double scale = 10;
  nevent++;
  int mevent = nevent/4;
  double v = memoryPattern[mevent%memoryPattern.size()];
  mf::LogVerbatim("memoryUsage") << "Event " << nevent
  	<< " leaks "<< v/scale << " Mbytes";
  if ( v > vsize ) {
    int leaksize = static_cast<int>(((v-vsize)/scale)*1048576);
    char* leak = new  char[leaksize];
    mf::LogPrint("memoryIncrease") << "Event " << mevent
  	<< " increases vsize by "<< ((v-vsize)/scale) << " Mbytes";
  }
  // DO NOT delete[] leak; the point is to increment vsize!

}  // MessageLoggerClient::analyze()

void  MemoryTestClient_B::initializeMemoryPattern(int pattern) {
  switch(pattern) {
    case 1:		// A general pattern
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(   3.1   );
	memoryPattern.push_back(   4.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(   1.7   );
	memoryPattern.push_back(   8.4   );
	memoryPattern.push_back(   3.4   );
	memoryPattern.push_back(  43.1   );
	memoryPattern.push_back(  17.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  47.9   );
	memoryPattern.push_back(   8.3   );
	memoryPattern.push_back(  56.3   );
	memoryPattern.push_back(   1.1   );
	memoryPattern.push_back(  19.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  22.0   );
	memoryPattern.push_back(   9.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  57.9   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  59.5   );
	memoryPattern.push_back(   4.1   );
	memoryPattern.push_back(   6.1   );
	memoryPattern.push_back(  61.5   );
	memoryPattern.push_back(   4.2   );
	memoryPattern.push_back(   6.3   );
    break;
    case 2:
	memoryPattern.push_back(  43.1   );
	memoryPattern.push_back(  17.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back( 119.5   );
	memoryPattern.push_back(  19.5   );
  break;
    case 3:
	memoryPattern.push_back(   3.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  47.9   );
	memoryPattern.push_back(   8.3   );
	memoryPattern.push_back(  56.3   );
	memoryPattern.push_back(   1.1   );
	memoryPattern.push_back(  19.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  22.0   );
	memoryPattern.push_back(   9.1   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  57.9   );
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(  17.1   );
    break;
    case 4:
	memoryPattern.push_back(  0   );
    break;
    default:
	memoryPattern.push_back(   2.1   );
	memoryPattern.push_back(   3.1   );
	memoryPattern.push_back(   4.1   );
	memoryPattern.push_back(   2.1   );
  }
}

}  // arttest


using arttest::MemoryTestClient_B;
DEFINE_ART_MODULE(MemoryTestClient_B);
