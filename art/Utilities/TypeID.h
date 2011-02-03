#ifndef art_Utilities_TypeID_h
#define art_Utilities_TypeID_h

/*----------------------------------------------------------------------

TypeID: A unique identifier for a C++ type.

The identifier is unique within an entire program, but can not be
persisted across invocations of the program.



----------------------------------------------------------------------*/
#include <iosfwd>
#include <typeinfo>
#include <string>
#include "art/Utilities/TypeIDBase.h"

namespace art {

  class TypeID : public TypeIDBase {
  public:

    TypeID() : TypeIDBase() {}

    TypeID(const TypeID& other) :
     TypeIDBase(other)
    { }

    explicit TypeID(const std::type_info& t) :
      TypeIDBase(t)
    { }

    // Copy assignment disallowed; see below.

    template <typename T>
    explicit TypeID(const T& t) :
      TypeIDBase(typeid(t))
    { }

    // Print out the name of the type, using the reflection class name.
    void print(std::ostream& os) const;

    std::string className() const;

    std::string userClassName() const;

    std::string friendlyClassName() const;

    bool hasDictionary() const;

  private:
    TypeID& operator=(const TypeID&); // not implemented

    static bool stripTemplate(std::string& theName);

    static bool stripNamespace(std::string& theName);

  };

  std::ostream& operator<<(std::ostream& os, const TypeID& id);
}
#endif /* art_Utilities_TypeID_h */

// Local Variables:
// mode: c++
// End:
