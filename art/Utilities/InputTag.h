#ifndef art_Utilities_InputTag_h
#define art_Utilities_InputTag_h

#include "art/Utilities/fwd.h"

#include <iosfwd>
#include <string>

namespace art {
  class InputTag;

  std::ostream& operator << (std::ostream& ost, InputTag const& tag);
  bool operator != (InputTag const & left, InputTag const & right);
}

class art::InputTag {
public:
  InputTag();

  // Create an InputTag by parsing the given string, which is
  // expected to be in colon-delimited form, fitting one of the
  // following patterns:
  //
  //   label
  //   label:instance
  //   label:instance:process
  InputTag(std::string const& s);
  InputTag(char const * s);

  // Create an InputTag with the given label, instance, and process
  // specificiations.
  InputTag(std::string const& label,
           std::string const& instance,
           std::string const& processName = std::string());

  // Create an InputTag with the given label, instance, and process
  // specifications.
  InputTag(char const*label,
           char const* instance,
           char const* processName="");


  // use compiler-generated copy c'tor, copy assignment, and d'tor

  std::string encode() const;

  std::string const& label() const {return label_;}
  std::string const& instance() const {return instance_;}
  ///an empty string means find the most recently produced
  ///product with the label and instance
  std::string const& process() const {return process_;}

  bool operator==(InputTag const& tag) const;

private:
  std::string label_;
  std::string instance_;
  std::string process_;

  // Helper function, to parse colon-separated initialization
  // string.
  void set_from_string_(std::string const& s);
};

#ifndef __GCCXML__
inline
art::InputTag::
InputTag()
  :
  label_(),
  instance_(),
  process_()
{
}

inline
art::InputTag::
InputTag(std::string const& label, std::string const& instance, std::string const& processName)
  :
  label_(label),
  instance_(instance),
  process_(processName)
{
}

inline
art::InputTag::
InputTag(char const* label, char const* instance, char const* processName)
  :
  label_(label),
  instance_(instance),
  process_(processName)
{
}

inline
art::InputTag::
InputTag(std::string const& s)
  :
  label_(),
  instance_(),
  process_()
{
  set_from_string_(s);
}

inline
art::InputTag::
InputTag(const char* s)
  :
  label_(),
  instance_(),
  process_()
{
  set_from_string_(s);
}

inline
bool
art::InputTag::
operator==(InputTag const& tag) const
{
  return (label_ == tag.label_)
    && (instance_ == tag.instance_)
    && (process_ == tag.process_);
}

inline
bool
art::
operator!=(InputTag const & left, InputTag const & right)
{
  return ! (left == right);
}
#endif /* __GCCXML__ */

#endif /* art_Utilities_InputTag_h */

// Local Variables:
// mode: c++
// End:
