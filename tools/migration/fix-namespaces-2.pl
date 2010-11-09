use strict;
use vars qw(%translations);

BEGIN { %translations = (
                         "artZ" => "art",
                        );
      }

foreach my $inc (reverse sort keys %translations) {
  while (s&(namespace\s+)\Q${inc}\E\b&${1}$translations{$inc}&g) {};
  while (s&${inc}::&$translations{$inc}::&g) {};
}

### Local Variables:
### mode: cperl
### End:
