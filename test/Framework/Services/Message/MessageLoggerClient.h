#ifndef Integration_MessageLoggerClient_h
#define Integration_MessageLoggerClient_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class MessageLoggerClient
  : public art::EDAnalyzer
{
public:
  explicit
    MessageLoggerClient( art::ParameterSet const & )
  { }

  virtual
    ~MessageLoggerClient()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif
