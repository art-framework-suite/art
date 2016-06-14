#include "cpp0x/utility"
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
  struct dictionary {
     std::string bsc1;
  std::vector<unsigned long> dummy1;
  std::vector<unsigned int> dummy2;
  std::vector<std::vector<unsigned int> > dummy2v;
  std::vector<long> dummy3;
  std::vector<int> dummy4;
  std::vector<std::vector<int> > dummy4v;
  std::vector<std::string> dummy5;
  std::vector<char> dummy6;
  std::vector<char*> dummy6p;
  std::vector<unsigned char> dummy7;
  std::vector<unsigned char*> dummy7p;
  std::vector<short> dummy8;
  std::vector<unsigned short> dummy9;
  std::vector<std::vector<unsigned short> > dummy9v;
  std::vector<double> dummy10;
  std::vector<long double> dummy11;
  std::vector<float> dummy12;
  // std::vector<bool> dummy13;
  std::vector<unsigned long long> dummy14;
  std::vector<long long> dummy15;
  std::vector<std::pair<std::string,double> > dummy16;
  std::vector<std::pair<unsigned int,double> > dummy16_1;
  std::list<int> dummy17;
  std::deque<int> dummy18;
  std::set<int> dummy19;
  std::set<std::string > dummy20;
  std::pair<unsigned long, unsigned long> dymmywp1;
  std::pair<unsigned int, unsigned int> dymmywp2;
  std::pair<unsigned int, int> dymmywp2_1;
  std::pair<unsigned int, unsigned long> dymmywp2_2;
  std::pair<unsigned short, unsigned short> dymmywp3;
  std::pair<int, int> dymmywp4;
  std::pair<unsigned int, bool> dymmywp5;
  std::pair<unsigned int, float> dymmywp6;
  std::pair<unsigned int, double> dymmywp6d;
  std::pair<double, double> dymmywp7;
  std::pair<unsigned long long, std::string > dymmywp8;
  std::pair<std::string,int> dummywp9;
  std::pair<std::string,double> dummywp10;
  std::pair<std::string,std::vector<std::pair<std::string,double> > > dummywp11;
  std::pair<std::string,std::vector<std::string > > dummywp12;
  std::pair<unsigned int,std::vector<unsigned int> > dymmywp13;
  std::pair<unsigned long,std::vector<unsigned long> > dymmywp14;
  std::pair<unsigned short,std::vector<unsigned short> > dymmywp15;

  std::map<unsigned long, unsigned long> dymmywm1;
  std::map<unsigned int, unsigned int> dymmywm2;
  std::map<unsigned int, int> dymmywm2_1;
  std::map<unsigned short, unsigned short> dymmywm3;
  std::map<int, int> dymmywm4;
  std::map<unsigned int, bool> dymmywm5;
  std::map<unsigned long, std::vector<unsigned long> > dymmywmv1;
  std::map<unsigned int, std::vector<unsigned int> > dymmywmv2;
  std::map<unsigned int,std::vector<std::pair<unsigned int,double> > >dymmywmv2_1;
  std::map<unsigned short, std::vector<unsigned short> > dymmypwmv3;
  std::map<unsigned int, float> dummyypwmv4;
  std::map<unsigned long long, std::string > dummyypwmv5;
  std::multimap<double, double> dummyypwmv6;
  std::map<std::string,int> dummyypwmv7;
  std::map<std::string,std::vector<std::pair<std::string,double> > > dummyypwmv8;
  std::map<int,std::pair<unsigned int,unsigned int> > dummyypwmv9;
  std::map<int,std::pair<unsigned long,unsigned long> > dummyypwmv10;
  std::map<std::string,std::vector<std::string > > dummyypwmv11;
  std::vector<char>::iterator itc;
  std::vector<short>::iterator its;
  std::vector<unsigned short>::iterator itus;
  std::vector<int>::iterator iti;
  std::vector<unsigned int>::iterator itui;
  std::vector<long>::iterator itl;
  std::vector<unsigned long>::iterator itul;
  std::vector<long long>::iterator itll;
  std::vector<unsigned long long>::iterator itull;
  std::vector<float>::iterator itf;
  std::vector<double>::iterator itd;
  std::vector<long double>::iterator itld;
  std::vector<std::string>::iterator itstring;
  std::vector<void *>::iterator itvp;
  std::allocator<char> achar;
  std::allocator<short> ashort;
  std::allocator<int> aint;
  std::allocator<double> adouble;
};
}
