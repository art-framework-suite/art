#include <string>

class Event;
class ToyConfig;


class MyAnalysis{

public:
  MyAnalysis ( ToyConfig& c );
  virtual ~MyAnalysis();

  void AnalyzeEvent( const Event& e);

private:

  std::string trackName;
  std::string digiName;

};

