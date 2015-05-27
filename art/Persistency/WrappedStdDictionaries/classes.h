#include "art/Persistency/Common/Wrapper.h"

#include <deque>
#include <list>
#include <map>
#include <set>
#include <utility>
#include <vector>

// ----------------------------------------------------------------------
//
// NOTE:
//
// The purpose of this file and of other class.h files is to force the
// instantiation of certain data types.  This intent is carried out today
// by providing a dummy data member declaration for each such type.
//
// It has been observed that a clearer expression of this intent is to
// provide instead, for each such type T, a C++ template instantiation
// of the form:
//   template class art::Wrapper<T>;
// If successful, this would obviate the need for any namespace scope
// and otherwise-useless dictionary type.
//
// However (using gcc 3.4.6), not all these proposed template
// instantiation statements compile.  There are two classes of failure,
// both occurring in the metaprogramming (in art::Wrapper<T>) that checks
// whether the wrapped type T has a T::swap(T&) member function.
//
// The first failure mode arises whenever T is an std::pair<,>.  The
// diagnostic says that std::pair<,> has no swap member (!).
//
// The second failure mode arises whenever T is a native type, such as
// int or double.  The diagnostic says that T is not an aggregate type.
//
// This should be revisited in the future to determine whether it is a
// fault of gcc 3.4.6, or of the metaprogramming in art::Wrapper<T>.
// The amount of overhead engendered by the two approaches should also
// be understood in order to decide which approach should be the norm.
//
// -- WEB  2010-06-25
//
// ----------------------------------------------------------------------

namespace {
  struct dictionary {
    art::Wrapper<std::vector<unsigned long> > dummy1;
    art::Wrapper<std::vector<unsigned int> > dummy2;
    art::Wrapper<std::vector<long> > dummy3;
    art::Wrapper<std::vector<int> > dummy4;
    art::Wrapper<std::vector<std::string> > dummy5;
    art::Wrapper<std::vector<char> > dummy6;
    art::Wrapper<std::vector<char*> > dummy6p;
    art::Wrapper<std::vector<unsigned char> > dummy7;
    art::Wrapper<std::vector<unsigned char*> > dummy7p;
    art::Wrapper<std::vector<short> > dummy8;
    art::Wrapper<std::vector<unsigned short> > dummy9;
    art::Wrapper<std::vector<std::vector<unsigned short> > > dummy9v;
    art::Wrapper<std::vector<double> > dummy10;
    art::Wrapper<std::vector<long double> > dummy11;
    art::Wrapper<std::vector<float> > dummy12;
    art::Wrapper<std::vector<bool> > dummy13;
    art::Wrapper<std::vector<unsigned long long> > dummy14;
    art::Wrapper<std::vector<long long> > dummy15;
    art::Wrapper<std::vector<std::pair<std::string,double> > > dummy16;
    art::Wrapper<std::vector<std::pair<unsigned int,double> > > dummy16_1;
    art::Wrapper<std::list<int> > dummy17;

    art::Wrapper<std::deque<int> > dummy18;

    art::Wrapper<std::set<int> > dummy19;

    art::Wrapper<std::pair<unsigned long, unsigned long> > dymmywp1;
    art::Wrapper<std::pair<unsigned int, unsigned int> > dymmywp2;
    art::Wrapper<std::pair<unsigned int, int> > dymmywp2_1;
    art::Wrapper<std::pair<unsigned short, unsigned short> > dymmywp3;
    art::Wrapper<std::pair<int, int> > dymmywp4;
    art::Wrapper<std::pair<unsigned int, bool> > dymmywp5;
    art::Wrapper<std::pair<unsigned int, float> > dymmywp6;
    art::Wrapper<std::pair<unsigned int, double> > dymmywp6d;
    art::Wrapper<std::pair<double, double> > dymmywp7;
    art::Wrapper<std::pair<unsigned long long, std::string > > dymmywp8;
    art::Wrapper<std::pair<std::string,int> > dummywp9;
    art::Wrapper<std::pair<std::string,double> > dummywp10;
    art::Wrapper<std::pair<std::string,std::vector<std::pair<std::string,double> > > > dummywp11;
    art::Wrapper<std::map<unsigned long, unsigned long> > dymmywm1;
    art::Wrapper<std::map<unsigned int, unsigned int> > dymmywm2;
    art::Wrapper<std::map<unsigned int, int> > dymmywm2_1;
    art::Wrapper<std::map<unsigned short, unsigned short> > dymmywm3;
    art::Wrapper<std::map<int, int> > dymmywm4;
    art::Wrapper<std::map<unsigned int, bool> > dymmywm5;
    art::Wrapper<std::map<unsigned long, std::vector<unsigned long> > > dymmywmv1;
    art::Wrapper<std::map<unsigned int, std::vector<unsigned int> > > dymmywmv2;
    art::Wrapper<std::map<unsigned int,std::vector<std::pair<unsigned int,double> > > >dymmywmv2_1;
    art::Wrapper<std::map<unsigned short, std::vector<unsigned short> > > dymmypwmv3;
    art::Wrapper<std::map<unsigned int, float> > dummyypwmv4;
    art::Wrapper<std::map<unsigned long long, std::string > > dummyypwmv5;
    art::Wrapper<std::multimap<double, double> > dummyypwmv6;
    art::Wrapper<std::map<std::string,bool> > dummyypwmv6a;
    art::Wrapper<std::map<std::string,int> > dummyypwmv7;
    art::Wrapper<std::map<std::string,std::vector<std::pair<std::string,double> > > > dummyypwmv8;
    art::Wrapper<std::map<int,std::pair<unsigned int,unsigned int> > > dummyypwmv9;
    art::Wrapper<std::map<int,std::pair<unsigned long,unsigned long> > > dummyypwmv10;

    art::Wrapper<unsigned long> dummyw1;
    art::Wrapper<unsigned int> dummyw2;
    art::Wrapper<long> dummyw3;
    art::Wrapper<int> dummyw4;
    art::Wrapper<std::string> dummyw5;
    art::Wrapper<char> dummyw6;
    art::Wrapper<unsigned char> dummyw7;
    art::Wrapper<short> dummyw8;
    art::Wrapper<unsigned short> dummyw9;
    art::Wrapper<double> dummyw10;
    art::Wrapper<long double> dummyw11;
    art::Wrapper<float> dummyw12;
    art::Wrapper<bool> dummyw13;
    art::Wrapper<unsigned long long> dummyw14;
    art::Wrapper<long long> dummyw15;
  };  // dictionary
}  // namespace
