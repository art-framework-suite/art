use strict;

use vars qw(%inc_translations);
BEGIN { %inc_translations = (
                             "art/Utilities/Exception.h" => "cetlib/exception.h",
                             "art/Utilities/EDMException.h" => "art/Utilities/Exception.h",
                            );
      }
foreach my $inc (sort keys %inc_translations) {
  s&^(\s*#include\s+["<])\Q$inc\E([">].*)$&${1}$inc_translations{$inc}${2}& and last;
}
while (s/artZ::Exception/cet::exception/g) {}
while (s/explainSelf/explain_self/g) {}
while (s/rootCause/root_cause/g) {}

### Local Variables:
### mode: cperl
### End:
