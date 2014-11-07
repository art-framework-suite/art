#ifndef art_Ntuple_tuple_helpers_h
#define art_Ntuple_tuple_helpers_h

#include <algorithm>
#include <regex>
#include <string>

namespace tupleUtils
{

  // enums used for filling tokens (see below in getTuple<>()
  // function)
  enum enum_type { STRICT, PERMISSIVE};

  //===============================================================
  // type conversion functions
  template<typename T> auto convertTo(const std::string& str) { return str; }

  // full specializations of free functions need to be
  // inline to avoid ODR violation
  template<> inline auto convertTo<double>  (const std::string& str) { return std::stod ( str ); }
  template<> inline auto convertTo<int>     (const std::string& str) { return std::stoi ( str ); }
  template<> inline auto convertTo<uint32_t>(const std::string& str) { return std::stoul( str ); }

  //===============================================================
  // TupleHelper is a helper struct, with function getTupleImpl. A
  // struct is needed because partial specialization of function
  // templates is not supported in C++11.  template<typename TUP,
  // std::size_t I>
  template<typename TUP,std::size_t I>
  struct TupleHelper{

    static void getTupleImpl( TUP& tmp,std::vector<std::string>::const_reverse_iterator strIt ){

      using result_t   = typename std::tuple_element<I,TUP>::type;
      std::get<I>(tmp) = convertTo<result_t>( *strIt );
      TupleHelper<TUP,I-1>::getTupleImpl( tmp, ++strIt );

    }

  };

  template<typename TUP>
  struct TupleHelper<TUP,0> {

    static void getTupleImpl(TUP& tmp, std::vector<std::string>::const_reverse_iterator strIt) {

      using result_t = typename std::tuple_element<0,TUP>::type;
      std::get<0>(tmp) = convertTo<result_t>( *strIt );

    }

  };

  //==============================================================================
  // getTuple is the function that is called by the user (e.g.):
  //
  //     auto tmp = getTuple<double,double,string>( some_string, regex_pattern );
  //
  // where "some_string" is converted into a tuple with the three
  // argument types listed above, based on the regex_pattern supplied.
  //
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // NB << It is the responsibility of the user to
  //    << ensure that the regex_pattern supplied will produce a
  //    << tokenized vector of the same size as the number of
  //    << template parameters!  The code below will throw if the
  //    << sizes do not match.
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  template<typename ... ARGS>
  inline auto getTuple( const std::string& str,
                        const std::string& regex_pattern,
                        const std::initializer_list<int>& matchList,
                        const enum_type PERM = STRICT ) {

    std::tuple<ARGS...> tmp;
    constexpr std::size_t SIZE = sizeof...(ARGS);

    std::vector<std::string> tokens;

    // The tokenizing is done using the std::regex library.  This
    // requires supplying the string to be matched, the regex pattern
    // to match against, and an initializer_list of integers that
    // correspond to the desired matches.

    std::copy_if( std::sregex_token_iterator(str.begin(),str.end(),std::regex( regex_pattern ), matchList ),
                  std::sregex_token_iterator(),
                  std::back_inserter( tokens ),
                  [](const auto& submatch){ return !submatch.str().empty();}
                  );

    // If the last fields are optional, setting the value of PERM to
    // PERMISSIVE allows the remaining required token entries to be
    // filled with empty strings.

    if ( PERM == PERMISSIVE && tokens.size() < SIZE )
      tokens.insert( tokens.end(), SIZE-tokens.size(), std::string() );

    if ( tokens.size() != SIZE ) {
      throw std::runtime_error("Tokenized string has " +
                               std::to_string( tokens.size() ) +
                               " elements when " +
                               std::to_string( SIZE ) +
                               " are required!" );
    }

    // Check invariant
    assert( tokens.size() == SIZE );

    // A REVERSE iterator on tokens is necessary as the TupleHelper is
    // called recursively, starting with the largest template
    // parameter index
    TupleHelper<decltype(tmp),SIZE-1>::getTupleImpl(tmp,tokens.crbegin());

    return tmp;
  }

}
#endif /* art_Ntuple_tuple_helpers_h */

// Local Variables:
// mode: c++
// End:
