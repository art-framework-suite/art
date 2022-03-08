#ifndef art_test_TestObjects_ToyProducts_h
#define art_test_TestObjects_ToyProducts_h

// ======================================================================
//
// EDProducts for testing purposes
//
// ======================================================================

#include "canvas/Persistency/Common/traits.h"
#include "cetlib/container_algorithms.h"

#include <array>
#include <cstdint>
#include <memory>
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
    IntProduct() = default;
    explicit IntProduct(int i) : value(i) {}

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

    int value{};
  };

  struct CompressedIntProduct {
    CompressedIntProduct() = default;
    explicit CompressedIntProduct(int i) : value(i) {}

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

    int value{};
  };

  struct Int16_tProduct {
    Int16_tProduct() = default;
    explicit Int16_tProduct(int16_t i, uint16_t j) : value(i), uvalue(j) {}

    int16_t value{};
    uint16_t uvalue{1};

    void
    aggregate(Int16_tProduct const& other)
    {
      value += other.value;
      uvalue += other.uvalue;
    }
  };

  struct DoubleProduct {
    DoubleProduct() = default;
    explicit DoubleProduct(double d) : value(d) {}

    void
    aggregate(DoubleProduct const& d)
    {
      value += d.value;
    }

    double value{2.2};
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
    StringProduct() = default;
    explicit StringProduct(const std::string& s) : name_(s) {}
    void
    aggregate(StringProduct const&)
    {}
    std::string name_{};
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
    Simple() = default;

    virtual ~Simple() noexcept = default;
    using key_type = int;

    key_type key{};
    double value{};

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
    SimpleDerived() = default;

    SimpleDerived(SimpleDerived const& other)
      : Simple(other), dummy_(other.dummy_)
    {}

    double dummy_{16.25};
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
    int data{};
    Sortable() = default;
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

  template <std::size_t N>
  struct IntArray {
    IntArray() = default;
    std::array<int, N> arr{{}};
    void
    aggregate(IntArray const& right)
    {
      for (std::size_t i{}; i < N; ++i) {
        arr[i] += right.arr[i];
      }
    }
  };

  // Test making a data product that has no default constructor and is
  // marked persistent="false" in the selection XML file.  We do not
  // create a class that explicitly deletes the default constructor
  // because ROOT's introspection mechanisms have a way of determining
  // this at compile time.
  struct NonPersistable {
    explicit NonPersistable(std::string const& n) : name{n} {}
    StringProduct name;
  };

  // Create type that encapsulates a pointer to NonPersistable--this
  // type *can* have a default constructor.
  struct PtrToNonPersistable {

    explicit PtrToNonPersistable() = default;
    explicit PtrToNonPersistable(std::string const& n)
      : np{new NonPersistable{n}}
    {}

    // Copying disabled
    PtrToNonPersistable(PtrToNonPersistable const&) = delete;
    PtrToNonPersistable& operator=(PtrToNonPersistable const&) = delete;

    // Move allowed
    PtrToNonPersistable(PtrToNonPersistable&& rhs) : np{rhs.np}
    {
      rhs.np = nullptr;
    }

    PtrToNonPersistable&
    operator=(PtrToNonPersistable&& rhs)
    {
      np = rhs.np;
      rhs.np = nullptr;
      return *this;
    }

    ~PtrToNonPersistable() noexcept
    {
      delete np;
      np = nullptr;
    }

    NonPersistable const* np{nullptr};
  };
}

// ======================================================================

#endif /* art_test_TestObjects_ToyProducts_h */

// Local Variables:
// mode: c++
// End:
