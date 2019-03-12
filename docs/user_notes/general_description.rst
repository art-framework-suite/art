This is the first release that removes the ROOT dependency from core art functionality. Two new packages have been introduced:

* art_root_io: provides ROOT functionality required for users (e.g. RootInput)
* critic: umbrella UPS product that sets up a consistent set of art, art_root_io, and gallery versions

Please consult the list of breaking changes to determine how your code should be adjusted to handle this migration.
