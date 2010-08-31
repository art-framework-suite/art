
class Event;
class ToyConfig;

class ToyGenerator{

public:
  ToyGenerator ( ToyConfig& );
  virtual ~ToyGenerator();

  void ProcessEvent( Event& e);
private:

  // Number of tracks to put into each event.
  int ntracks;

};

