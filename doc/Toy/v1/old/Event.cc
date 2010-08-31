#include "Event.hh"

Event::Event(){}

Event::~Event(){}

void Event::Put( std::string& s, void* val){
  M[s] = val;
}

const void* Event::Get( std::string& s) const {
  return M[s];
}
