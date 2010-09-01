#include <vector>
#include <string>

class ToyMCHit;

class ToyDigi{

public:
  ToyDigi ( int hitid,
	    const ToyMCHit& hit,
	    const std::string& collection );
  virtual ~ToyDigi();

  const ToyMCHit& parentHit() const;

  int channelID()  const { return _chan;}
  int tdc()        const { return _itdc;}
  int adc()        const { return _iadc;}

private:

  // Unique identifier of the hit from which this digi was made
  // is an index within a collection.
  int _hitid;
  std::string _collection;

  // Channel ID.
  int _chan;

  // Whatever it is we measure.
  int _iadc;
  int _itdc;

  // Not persistent.  Needs to be recomputed on demand during readback.
  ToyMCHit const * _hit;

};

