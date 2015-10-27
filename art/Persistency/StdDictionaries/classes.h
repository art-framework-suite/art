#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

template class std::vector<unsigned long>;
template class std::vector<unsigned int>;
template class std::vector<std::vector<unsigned int>>;
template class std::vector<long>;
template class std::vector<int>;
template class std::vector<std::vector<int>>;
template class std::vector<std::string>;
template class std::vector<char>;
template class std::vector<char*>;
template class std::vector<unsigned char>;
template class std::vector<unsigned char*>;
template class std::vector<short>;
template class std::vector<unsigned short>;
template class std::vector<std::vector<unsigned short>>;
template class std::vector<double>;
template class std::vector<long double>;
template class std::vector<float>;
template class std::vector<unsigned long long>;
template class std::vector<long long>;
template class std::vector<std::pair<std::string, double>>;
template class std::vector<std::pair<unsigned int, double>>;
template class std::list<int>;
template class std::deque<int>;
template class std::set<int>;
template class std::set<std::string>;
template class std::pair<unsigned long, unsigned long>;
template class std::pair<unsigned int, unsigned int>;
template class std::pair<unsigned int, int>;
template class std::pair<unsigned int, unsigned long>;
template class std::pair<unsigned short, unsigned short>;
template class std::pair<int, int>;
template class std::pair<unsigned int, bool>;
template class std::pair<unsigned int, float>;
template class std::pair<unsigned int, double>;
template class std::pair<double, double>;
template class std::pair<unsigned long long, std::string>;
template class std::pair<std::string, int>;
template class std::pair<std::string, double>;
template class std::pair<std::string, std::vector<std::pair<std::string, double>>>;
template class std::pair<std::string, std::vector<std::string>>;
template class std::pair<unsigned int, std::vector<unsigned int>>;
template class std::pair<unsigned long, std::vector<unsigned long>>;
template class std::pair<unsigned short, std::vector<unsigned short>>;
template class std::map<unsigned long, unsigned long>;
template class std::map<unsigned int, unsigned int>;
template class std::map<unsigned int, int>;
template class std::map<unsigned short, unsigned short>;
template class std::map<int, int>;
template class std::map<unsigned int, bool>;
template class std::map<unsigned long, std::vector<unsigned long>>;
template class std::map<unsigned int, std::vector<unsigned int>>;
template class std::map<unsigned int, std::vector<std::pair<unsigned int, double>>>;
template class std::map<unsigned short, std::vector<unsigned short>>;
template class std::map<unsigned int, float>;
template class std::map<unsigned long long, std::string>;
template class std::multimap<double, double>;
template class std::map<std::string, int>;
template class std::map<std::string, std::vector<std::pair<std::string, double>>>;
template class std::map<int, std::pair<unsigned int, unsigned int>>;
template class std::map<int, std::pair<unsigned long, unsigned long>>;
template class std::map<std::string, std::vector<std::string>>;
#if 0
template class std::vector<char>::iterator itc;
template class std::vector<short>::iterator its;
template class std::vector<unsigned short>::iterator itus;
template class std::vector<int>::iterator iti;
template class std::vector<unsigned int>::iterator itui;
template class std::vector<long>::iterator itl;
template class std::vector<unsigned long>::iterator itul;
template class std::vector<long long>::iterator itll;
template class std::vector<unsigned long long>::iterator itull;
template class std::vector<float>::iterator itf;
template class std::vector<double>::iterator itd;
template class std::vector<long double>::iterator itld;
template class std::vector<std::string>::iterator itstring;
template class std::vector<void*>::iterator itvp;
#endif // 0
template class std::allocator<char>;
template class std::allocator<short>;
template class std::allocator<int>;
template class std::allocator<double>;
