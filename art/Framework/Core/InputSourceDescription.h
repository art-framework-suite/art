#ifndef art_Framework_Core_InputSourceDescription_h
#define art_Framework_Core_InputSourceDescription_h

/*----------------------------------------------------------------------

InputSourceDescription: This is an "argument pack" structure, used to
pass a collection of related arguments to the constructors of InputSources.
InputSourceDescriptions should *not* be kept as data members, or stored
in any way.

InputSourceDescriptions should generally be passed by non-const reference,
so that the non-const reference data members can be used correctly.

----------------------------------------------------------------------*/

namespace art {
  class ActivityRegistry;
  class UpdateOutputCallbacks;
  class ModuleDescription;

  struct InputSourceDescription {
    InputSourceDescription(ModuleDescription const&,
                           UpdateOutputCallbacks&,
                           ActivityRegistry const&);

    InputSourceDescription(InputSourceDescription const&) = delete;
    InputSourceDescription(InputSourceDescription&&) = delete;

    ModuleDescription const& moduleDescription;
    UpdateOutputCallbacks& productRegistry;
    ActivityRegistry const& activityRegistry;
  };

  inline InputSourceDescription::InputSourceDescription(
    ModuleDescription const& md,
    UpdateOutputCallbacks& preg,
    ActivityRegistry const& areg)
    : moduleDescription(md), productRegistry(preg), activityRegistry(areg)
  {}

} // namespace art

#endif /* art_Framework_Core_InputSourceDescription_h */

// Local Variables:
// mode: c++
// End:
