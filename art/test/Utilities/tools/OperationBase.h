#ifndef art_test_Utilities_tools_OperationBase_h
#define art_test_Utilities_tools_OperationBase_h

namespace arttest {
  class OperationBase {
  public:
    virtual ~OperationBase() noexcept = default;

    void
    adjustNumber(int& i) const
    {
      do_adjustNumber(i);
    }

  private:
    virtual void do_adjustNumber(int&) const = 0;
  };
}

#endif /* art_test_Utilities_tools_OperationBase_h */

// Local variables:
// mode: c++
// End:
