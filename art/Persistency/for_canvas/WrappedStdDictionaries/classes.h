#include "canvas/Persistency/Common/Wrapper.h"

#include <deque>
#include <list>
#include <map>
#include <set>
#include <utility>
#include <vector>

template class art::Wrapper<std::vector<unsigned long>>;
template class art::Wrapper<std::vector<unsigned int>>;
template class art::Wrapper<std::vector<long>>;
template class art::Wrapper<std::vector<int>>;
template class art::Wrapper<std::vector<std::string>>;
template class art::Wrapper<std::vector<char>>;
template class art::Wrapper<std::vector<char*>>;
template class art::Wrapper<std::vector<unsigned char>>;
template class art::Wrapper<std::vector<unsigned char*>>;
template class art::Wrapper<std::vector<short>>;
template class art::Wrapper<std::vector<unsigned short>>;
template class art::Wrapper<std::vector<std::vector<unsigned short>>>;
template class art::Wrapper<std::vector<double>>;
template class art::Wrapper<std::vector<long double>>;
template class art::Wrapper<std::vector<float>>;
template class art::Wrapper<std::vector<bool>>;
template class art::Wrapper<std::vector<unsigned long long>>;
template class art::Wrapper<std::vector<long long>>;
template class art::Wrapper<std::vector<std::pair<std::string, double>>>;
template class art::Wrapper<std::vector<std::pair<unsigned int, double>>>;
template class art::Wrapper<std::list<int>>;
template class art::Wrapper<std::deque<int>>;
template class art::Wrapper<std::set<int>>;
template class art::Wrapper<std::pair<unsigned long, unsigned long>>;
template class art::Wrapper<std::pair<unsigned int, unsigned int>>;
template class art::Wrapper<std::pair<unsigned int, int>>;
template class art::Wrapper<std::pair<unsigned short, unsigned short>>;
template class art::Wrapper<std::pair<int, int>>;
template class art::Wrapper<std::pair<unsigned int, bool>>;
template class art::Wrapper<std::pair<unsigned int, float>>;
template class art::Wrapper<std::pair<unsigned int, double>>;
template class art::Wrapper<std::pair<double, double>>;
template class art::Wrapper<std::pair<unsigned long long, std::string>>;
template class art::Wrapper<std::pair<std::string, int>>;
template class art::Wrapper<std::pair<std::string, double>>;
template class art::Wrapper<std::pair<std::string, std::vector<std::pair<std::string, double>>>>;
template class art::Wrapper<std::map<unsigned long, unsigned long>>;
template class art::Wrapper<std::map<unsigned int, unsigned int>>;
template class art::Wrapper<std::map<unsigned int, int>>;
template class art::Wrapper<std::map<unsigned short, unsigned short>>;
template class art::Wrapper<std::map<int, int>>;
template class art::Wrapper<std::map<unsigned int, bool>>;
template class art::Wrapper<std::map<unsigned long, std::vector<unsigned long>>>;
template class art::Wrapper<std::map<unsigned int, std::vector<unsigned int>>>;
template class art::Wrapper<std::map<unsigned int, std::vector<std::pair<unsigned int, double>>>>;
template class art::Wrapper<std::map<unsigned short, std::vector<unsigned short>>>;
template class art::Wrapper<std::map<unsigned int, float>>;
template class art::Wrapper<std::map<unsigned long long, std::string>>;
template class art::Wrapper<std::multimap<double, double>>;
template class art::Wrapper<std::map<std::string, bool>>;
template class art::Wrapper<std::map<std::string, int>>;
template class art::Wrapper<std::map<std::string, std::vector<std::pair<std::string, double>>>>;
template class art::Wrapper<std::map<int, std::pair<unsigned int, unsigned int>>>;
template class art::Wrapper<std::map<int, std::pair<unsigned long, unsigned long>>>;
template class art::Wrapper<unsigned long>;
template class art::Wrapper<unsigned int>;
template class art::Wrapper<long>;
template class art::Wrapper<int>;
template class art::Wrapper<std::string>;
template class art::Wrapper<char>;
template class art::Wrapper<unsigned char>;
template class art::Wrapper<short>;
template class art::Wrapper<unsigned short>;
template class art::Wrapper<double>;
template class art::Wrapper<long double>;
template class art::Wrapper<float>;
template class art::Wrapper<bool>;
template class art::Wrapper<unsigned long long>;
template class art::Wrapper<long long>;

