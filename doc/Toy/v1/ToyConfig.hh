#include <string>
#include <vector>

class ToyConfig{

public:
  ToyConfig();
  ~ToyConfig();


  int          getInt  ( std::string key) const;
  double      getDouble( std::string key) const;
  std::string getString( std::string key) const;

  const std::vector<int>&         getVectorInt( std::string key)    const;
  const std::vector<double>&      getVectorDouble( std::string key) const;
  const std::vector<std::string>& getVectorString( std::string key) const;

private:



};

