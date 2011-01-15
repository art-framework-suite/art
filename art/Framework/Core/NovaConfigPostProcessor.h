#ifndef NovaConfigPostProcessor_h
#define NovaConfigPostProcessor_h

#include <string>

namespace fhicl {
   class intermediate_table;
}

class NovaConfigPostProcessor {
 public:
   NovaConfigPostProcessor();
   void apply(fhicl::intermediate_table &raw_config) const;

   void source(std::string const &source) { source_ = source; }
   void tFileName(std::string const &tFileName)
   { tFileName_ = tFileName; }
   void output(std::string const &output) { output_ = output; }
   void nevts(int nevts) {nevts_ = nevts; wantNevts_ = true; }
   void startEvt(int startEvt) {startEvt_ = startEvt; wantStartEvt_ = true; }
   void skipEvts(int skipEvts) {skipEvts_ = skipEvts; wantSkipEvts_ = true; }
 private:

   void applySource(fhicl::intermediate_table &raw_config) const;
   void applyOutput(fhicl::intermediate_table &raw_config) const;
   void applyTFileName(fhicl::intermediate_table &raw_config) const;

   std::string source_;
   std::string tFileName_;
   std::string output_;
   int nevts_;
   int startEvt_;
   int skipEvts_;
   bool wantNevts_;
   bool wantStartEvt_;
   bool wantSkipEvts_;
};
#endif
