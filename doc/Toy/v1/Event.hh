#include <string>
#include <map>

class Event{

public:

  Event();
  virtual ~Event();

  void  Put( std::string& key, void* val);
  const void* Get( std::string& key) const ;

private:

  // Mutable so that I can be lazy in the Get method
  // and still make it a const method.
  mutable std::map<std::string,void*> M;

};


