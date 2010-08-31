#include "MyAnalysis.hh"

#include "Event.hh"
#include "ToyConfig.hh"
#include "ToyMCTrack.hh"
#include "ToyDigi.hh"

#include <vector>
#include <string>

using std::vector;
using std::string;

MyAnalysis::MyAnalysis( ToyConfig& c ){
  trackName = c.getString("trackName");
  digiName  = c.getString("digiName");
}

MyAnalysis::~MyAnalysis(){
}

void MyAnalysis::AnalyzeEvent( const Event& evt ){


  // Get the input collections.
  const vector<ToyMCTrack>* tracks = (const vector<ToyMCTrack>*) evt.Get(trackName);
  const vector<ToyDigi>*    digis  = (const vector<ToyDigi>*)    evt.Get(digiName);

  // Loop over tracks.
  {
    vector<ToyMCTrack>::const_iterator b = tracks->begin();
    vector<ToyMCTrack>::const_iterator e = tracks->end();
    while (b != e){
      const ToyMCTrack& track = *b;
      // Make histograms.
      ++b;
    }
  }

  // Loop over digis
  {
    vector<ToyDigi>::const_iterator b = digis->begin();
    vector<ToyDigi>::const_iterator e = digis->end();
    while (b != e){
      const ToyDigi& digi = *b;
      // Make histograms.

      // Access parent hit.
      const ToyMCHit& hit = digi.parentHit();
      // Make more histograms.

      ++b;
    }
  }
}
