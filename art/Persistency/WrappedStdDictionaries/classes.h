#include "art/Persistency/Common/Wrapper.h"

#include <deque>
#include <list>
#include <map>
#include <set>
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
//   template class edm::Wrapper<T>;
// If successful, this would obviate the need for any namespace scope
// and otherwise-useless dictionary type.
//
// However (using gcc 3.4.6), not all these proposed template
// instantiation statements compile.  There are two classes of failure,
// both occurring in the metaprogramming (in edm::Wrapper<T>) that checks
// whether the wrapped type T has a T::swap(T&) member function.
//
// The first failure mode arises whenever T is an std::pair<,>.  The
// diagnostic says that std::pair<,> has no swap member (!).
//
// The second failure mode arises whenever T is a native type, such as
// int or double.  The diagnostic says that T is not an aggregate type.
//
// This should be revisited in the future to determine whether it is a
// fault of gcc 3.4.6, or of the metaprogramming in edm::Wrapper<T>.
// The amount of overhead engendered by the two approaches should also
// be understood in order to decide which approach should be the norm.
//
// -- WEB  2010-06-25
//
// ----------------------------------------------------------------------

namespace {
  struct dictionary {
    edm::Wrapper<std::vector<unsigned long> > dummy1;
    edm::Wrapper<std::vector<unsigned int> > dummy2;
    edm::Wrapper<std::vector<long> > dummy3;
    edm::Wrapper<std::vector<int> > dummy4;
    edm::Wrapper<std::vector<std::string> > dummy5;
    edm::Wrapper<std::vector<char> > dummy6;
    edm::Wrapper<std::vector<char*> > dummy6p;
    edm::Wrapper<std::vector<unsigned char> > dummy7;
    edm::Wrapper<std::vector<unsigned char*> > dummy7p;
    edm::Wrapper<std::vector<short> > dummy8;
    edm::Wrapper<std::vector<unsigned short> > dummy9;
    edm::Wrapper<std::vector<std::vector<unsigned short> > > dummy9v;
    edm::Wrapper<std::vector<double> > dummy10;
    edm::Wrapper<std::vector<long double> > dummy11;
    edm::Wrapper<std::vector<float> > dummy12;
    edm::Wrapper<std::vector<bool> > dummy13;
    edm::Wrapper<std::vector<unsigned long long> > dummy14;
    edm::Wrapper<std::vector<long long> > dummy15;
    edm::Wrapper<std::vector<std::pair<std::basic_string<char>,double> > > dummy16;
    edm::Wrapper<std::vector<std::pair<unsigned int,double> > > dummy16_1;
    edm::Wrapper<std::list<int> > dummy17;

    edm::Wrapper<std::deque<int> > dummy18;

    edm::Wrapper<std::set<int> > dummy19;

    edm::Wrapper<std::pair<unsigned long, unsigned long> > dymmywp1;
    edm::Wrapper<std::pair<unsigned int, unsigned int> > dymmywp2;
    edm::Wrapper<std::pair<unsigned int, int> > dymmywp2_1;
    edm::Wrapper<std::pair<unsigned short, unsigned short> > dymmywp3;
    edm::Wrapper<std::pair<int, int> > dymmywp4;
    edm::Wrapper<std::pair<unsigned int, bool> > dymmywp5;
    edm::Wrapper<std::pair<unsigned int, float> > dymmywp6;
    edm::Wrapper<std::pair<unsigned int, double> > dymmywp6d;
    edm::Wrapper<std::pair<double, double> > dymmywp7;
    edm::Wrapper<std::pair<unsigned long long, std::basic_string<char> > > dymmywp8;
    edm::Wrapper<std::pair<std::basic_string<char>,int> > dummywp9;
    edm::Wrapper<std::pair<std::basic_string<char>,double> > dummywp10;
    edm::Wrapper<std::pair<std::basic_string<char>,std::vector<std::pair<std::basic_string<char>,double> > > > dummywp11;
    edm::Wrapper<std::map<unsigned long, unsigned long> > dymmywm1;
    edm::Wrapper<std::map<unsigned int, unsigned int> > dymmywm2;
    edm::Wrapper<std::map<unsigned int, int> > dymmywm2_1;
    edm::Wrapper<std::map<unsigned short, unsigned short> > dymmywm3;
    edm::Wrapper<std::map<int, int> > dymmywm4;
    edm::Wrapper<std::map<unsigned int, bool> > dymmywm5;
    edm::Wrapper<std::map<unsigned long, std::vector<unsigned long> > > dymmywmv1;
    edm::Wrapper<std::map<unsigned int, std::vector<unsigned int> > > dymmywmv2;
    edm::Wrapper<std::map<unsigned int,std::vector<std::pair<unsigned int,double> > > >dymmywmv2_1;
    edm::Wrapper<std::map<unsigned short, std::vector<unsigned short> > > dymmypwmv3;
    edm::Wrapper<std::map<unsigned int, float> > dummyypwmv4;
    edm::Wrapper<std::map<unsigned long long, std::basic_string<char> > > dummyypwmv5;
    edm::Wrapper<std::multimap<double, double> > dummyypwmv6;
    edm::Wrapper<std::map<std::basic_string<char>,bool> > dummyypwmv6a;
    edm::Wrapper<std::map<std::basic_string<char>,int> > dummyypwmv7;
    edm::Wrapper<std::map<std::basic_string<char>,std::vector<std::pair<std::basic_string<char>,double> > > > dummyypwmv8;
    edm::Wrapper<std::map<int,std::pair<unsigned int,unsigned int> > > dummyypwmv9;
    edm::Wrapper<std::map<int,std::pair<unsigned long,unsigned long> > > dummyypwmv10;

    edm::Wrapper<unsigned long> dummyw1;
    edm::Wrapper<unsigned int> dummyw2;
    edm::Wrapper<long> dummyw3;
    edm::Wrapper<int> dummyw4;
    edm::Wrapper<std::string> dummyw5;
    edm::Wrapper<char> dummyw6;
    edm::Wrapper<unsigned char> dummyw7;
    edm::Wrapper<short> dummyw8;
    edm::Wrapper<unsigned short> dummyw9;
    edm::Wrapper<double> dummyw10;
    edm::Wrapper<long double> dummyw11;
    edm::Wrapper<float> dummyw12;
    edm::Wrapper<bool> dummyw13;
    edm::Wrapper<unsigned long long> dummyw14;
    edm::Wrapper<long long> dummyw15;
  };  // dictionary
}  // namespace
