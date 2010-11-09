use strict;
use vars qw(%translations);

BEGIN { %translations = (
                         cms => "artZ",
                         edm => "art",
                         edmNew => "artNew",
                         edmplugin => "artplugin",
                         edmplugintest => "artplugintest",
                         edmtest => "arttest",
                         edmtestprod => "arttestprod",
                        );
      }

foreach my $inc (reverse sort keys %translations) {
  while (s&(namespace\s+)\Q${inc}\E\b&${1}$translations{$inc}&g) {};
  while (s&${inc}::&$translations{$inc}::&g) {};
}

### Local Variables:
### mode: cperl
### End:
