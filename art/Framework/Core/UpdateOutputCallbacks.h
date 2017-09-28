#ifndef art_Framework_Core_UpdateOutputCallbacks_h
#define art_Framework_Core_UpdateOutputCallbacks_h
// vim: set sw=2 expandtab :

// =====================================================================
// UpdateOutputCallbacks
//
// UpdateOutputCallbacks is a class that contains a list of callbacks
// that can be invoked to update output modules.  For example, if an
// input file is opened and an output module needs to be updated
// accordingly, the output module (or EndPathExecutor) can register a
// callback with an object of this class.  The input source's
// reference to that object can then invoke the callbacks whenever a
// new input file is opened.
//
// This class originated from the MasterProductRegistry which, in
// addition to owning descriptions of all products, also had a list of
// callbacks to invoke whenever that list of product descriptions was
// updated.  All product descriptions are now owned by the input
// files/source, the event processor, and any output modules. The
// callback invocations are done through the UpdateOutputCallbacks
// class to provide the ability to synchronize product descriptions
// between the input/event processor and the output.
//
// While the only currently-supported callback signature is of type
// void(ProductTables const&), other callback signatures could be
// beneficial.  When we get to that point, it would make sense to
// introduce a different model where users of this class would provide
// subclasses that inherent from a common base class (e.g.):
//
//   outputCallbacks.registerCallback<FileBlockPacket>(...);
//   outputCallbacks.registerCallback<ProductTablesPacket>(...);
//
// where both FileBlockPacket and ProductTablesPacket inherit from a
// base class of type (e.g.) IOPacket.  The relevant callback would be
// invoked via something like:
//
//   template <typename T, typename... Args>
//   void invoke(Args&&... args) {
//     auto& callback = dynamic_cast<T&>(entry_in_callback_list);
//     callback.invoke(std::forward<Args>(args)...);
//   }
//
// where 'entry_in_callback_list' refers to a pointer of type IOPacket
// corresponding to the desired callback to invoke.
// =====================================================================

#include <functional>
#include <vector>

namespace art {

  class ProductTables;
  using ProductListUpdatedCallback = std::function<void(ProductTables const&)>;

  class UpdateOutputCallbacks {
  public:

    explicit UpdateOutputCallbacks() = default;
    UpdateOutputCallbacks(UpdateOutputCallbacks const&) = delete;

    void registerCallback(ProductListUpdatedCallback cb);

    void invoke(ProductTables const&);

  private:
    std::vector<ProductListUpdatedCallback> callbacks_{};
  };

} // namespace art

#endif /* art_Framework_Core_UpdateOutputCallbacks_h */

// Local Variables:
// mode: c++
// End:
