use strict;
use vars qw(%translations);

BEGIN { %translations = (
                         artVersionTest => "art_Version_Test",
                         artVersion => "art_Version",
                         artUtilitiesTest => "art_Utilities_Test",
                         artUtilities => "art_Utilities",
                        );
      }

foreach my $inc (keys %translations) {
  while (s&${inc}&$translations{$inc}&g) {};
}

### Local Variables:
### mode: cperl
### End:
