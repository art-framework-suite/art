class Event;
class ToyConfig;

class ToyDigitizer{

public:
  ToyDigitizer ( ToyConfig& c );
  virtual ~ToyDigitizer();

  void ProcessEvent( Event& e);

private:

  // Minimum pulseheight for making a digi.
  double threshold;

};

