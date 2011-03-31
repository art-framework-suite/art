use strict;

use vars qw(%inc_translations);

BEGIN { %inc_translations =
	    ( "art/Framework/Services/Registry/Service.h" => "art/Framework/Services/Registry/ServiceHandle.h"
	      );
	}

foreach my $inc (sort keys %inc_translations) {
  s&^(\s*#include\s+["<])\Q$inc\E([">].*)$&${1}$inc_translations{$inc}${2}& and last;
}

while (s/Service</ServiceHandle</g) {};

### Local Variables:
### mode: cperl
### End:
