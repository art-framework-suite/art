use strict;
use vars qw(%translations);

BEGIN { %translations = (
                         cms => "artZ",
                         edm => "art",
                         edmNew => "artNew",
                         edmplugin => "artplugin",
                         edmplugintest => "artplugintest",
                         edmtest => "arttest",
                         edmtest => "arttestprod",
                        );
      }

foreach my $inc (sort keys %translations) {
  s&(namespace\s+)\Q$inc\E\b&${1}$translations{$inc}&g;
  s&$inc::&$translations{$inc}::&g;
}

### Local Variables:
### mode: cperl
### End:
