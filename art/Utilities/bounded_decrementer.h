#ifndef art_Utilities_bounded_decrementer_h
#define art_Utilities_bounded_decrementer_h

// ====================================================================
// bounded_decrementer
//
// Given an initial value, the bounded_decrementer acts as a
// decrementable integer (with pre- and post-fix decrement operations)
// so long as the internal value is positive.  Once the internal value
// reaches 0, all subsequent decrement operations are no-ops.
//
// This utility is helpful when making comparisons to 0 using its
// implicit conversion to int (e.g.):
//
//   if (decrementer_ == 0) {
//     return ...;
//   }
//   --decrementer_;
//
// If the initial value is a positive integer n, the 'return'
// statement will be executed after --decrementer_ has been invoked n
// times.  If, however, the initial value is negative (such as
// specifying '-1' to indicate infinity), the internal value will
// never reach 0, and the 'return' statement will never be executed.
// ===================================================================

namespace art {
  class bounded_decrementer {
  public:
    constexpr explicit bounded_decrementer(int const count) : count_{count} {}
    static constexpr bounded_decrementer
    unlimited()
    {
      return bounded_decrementer{-1};
    }
    constexpr
    operator int() const noexcept
    {
      return count_;
    }

    // pre-increment
    constexpr bounded_decrementer&
    operator--() noexcept
    {
      if (count_ > 0) {
        --count_;
      }
      return *this;
    }

    // post-increment
    constexpr bounded_decrementer
    operator--(int) noexcept
    {
      auto old = *this;
      operator--();
      return old;
    }

  private:
    int count_;
  };
}

#endif /* art_Utilities_bounded_decrementer_h */

// Local Variables:
// mode: c++
// End:
