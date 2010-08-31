#include "ToyDigi.hh"

using std::vector;

ToyDigi::ToyDigi ( int hitid,
		   const ToyMCHit& hit,
		   const std::string& collection ):
  _hitid(hitid),
  _hit(&hit),
  _collection(collection){

  // _iadc = computed from hit.pulseHeight();
  // _itdc = computed from hit.time() or something like that.

}

const ToyMCHit& ToyDigi::parentHit() const{
  if ( _hit != 0 ) return *_hit;

  // ( hitid and collection ) to find the hit within the event.

  return *_hit;
}


ToyDigi::~ToyDigi(){
}
