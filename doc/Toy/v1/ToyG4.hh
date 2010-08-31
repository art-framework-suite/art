
class Event;
class ToyConfig;

class ToyG4{

public:
  ToyG4 ( ToyConfig& c );
  virtual ~ToyG4();

  void ProcessEvent( Event& e);

private:

  // Number of hits to put on each track.
  int nhits;


};

