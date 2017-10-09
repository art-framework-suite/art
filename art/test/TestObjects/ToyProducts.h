#ifndef art_test_TestObjects_ToyProducts_h
#define art_test_TestObjects_ToyProducts_h

// ======================================================================
//
// EDProducts for testing purposes
//
// ======================================================================

#include "canvas/Persistency/Common/traits.h"
#include "cetlib/container_algorithms.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace arttest {

  struct DummyProduct {
    void
    aggregate(DummyProduct const&) const
    {}
  };

  struct IntProduct {
    explicit IntProduct(int i = 0) : value(i) {}

    IntProduct&
    operator+=(IntProduct const& other)
    {
      value += other.value;
      return *this;
    }

    void
    aggregate(IntProduct const& other)
    {
      (void)operator+=(other);
    }

    int value;
  };

  struct CompressedIntProduct {
    explicit CompressedIntProduct(int i = 0) : value(i) {}

    CompressedIntProduct&
    operator+=(CompressedIntProduct const& other)
    {
      value += other.value;
      return *this;
    }

    void
    aggregate(CompressedIntProduct const& other)
    {
      (void)operator+=(other);
    }

    int value;
  };

  struct Int16_tProduct {
    explicit Int16_tProduct(int16_t i = 0, uint16_t j = 1) : value(i), uvalue(j)
    {}

    int16_t value;
    uint16_t uvalue;

    void
    aggregate(Int16_tProduct const& other)
    {
      value += other.value;
      uvalue += other.uvalue;
    }
  };

  struct DoubleProduct {
    explicit DoubleProduct(double d = 2.2) : value(d) {}

    void
    aggregate(DoubleProduct const& d)
    {
      value += d.value;
    }

    double value;
  };

  inline DoubleProduct
  operator+(DoubleProduct const& left, DoubleProduct const right)
  {
    return DoubleProduct(left.value + right.value);
  }

  inline DoubleProduct&
  operator+=(DoubleProduct& left, DoubleProduct const& right)
  {
    left.aggregate(right);
    return left;
  }

  inline DoubleProduct&
  operator+=(DoubleProduct& left, double right)
  {
    left.value += right;
    return left;
  }

  struct StringProduct {
    StringProduct() : name_() {}
    explicit StringProduct(const std::string& s) : name_(s) {}
    void
    aggregate(StringProduct const&)
    {}
    std::string name_;
  };

  inline bool
  operator==(StringProduct const& left, StringProduct const& right)
  {
    return left.name_ == right.name_;
  }

  inline std::ostream&
  operator<<(std::ostream& os, StringProduct const& s)
  {
    return os << s.name_;
  }

  struct Simple {
    Simple() : key(0), value(0.0) {}
    virtual ~Simple() noexcept = default;
    typedef int key_type;

    key_type key;
    double value;

    key_type
    id() const
    {
      return key;
    }
    virtual double
    dummy() const
    {
      return -3.14;
    }
    virtual Simple*
    clone() const
    {
      return new Simple(*this);
    }
  };

  inline bool
  operator==(Simple const& a, Simple const& b)
  {
    return (a.key == b.key && a.value == b.value);
  }

  inline bool
  operator<(Simple const& a, Simple const& b)
  {
    return a.key < b.key;
  }

  struct SimpleDerived : public Simple {
    SimpleDerived() : Simple(), dummy_(16.25) {}

    SimpleDerived(SimpleDerived const& other)
      : Simple(other), dummy_(other.dummy_)
    {}

    double dummy_;
    double
    dummy() const override
    {
      return dummy_;
    }
    SimpleDerived*
    clone() const override
    {
      return new SimpleDerived(*this);
    }
  };

  struct Sortable {
    int data;
    Sortable() : data(0) {}
    explicit Sortable(int i) : data(i) {}
    void
    aggregate(Sortable const&) const
    {}
  };

  inline bool
  operator==(Sortable const& a, Sortable const& b)
  {
    return (a.data == b.data);
  }

  inline bool
  operator<(Sortable const& a, Sortable const& b)
  {
    return a.data < b.data;
  }

  using VSimpleProduct = std::vector<Simple>;

  struct Hit {
    Hit() : id(-1) {}
    Hit(size_t id) : id(id) {}
    size_t id;
    void
    aggregate(Hit const&) const
    {}
  };

  struct Track {
    Track() : id(-1) {}
    Track(size_t id) : id(id) {}
    size_t id;
    void
    aggregate(Track const&) const
    {}
  };
}

  // ======================================================================

#endif /* art_test_TestObjects_ToyProducts_h */

// Local Variables:
// mode: c++
// End:
