# Usage (e.g.): find -L -regex ".*\.\(h\|hh\|cc\|cpp\|cxx\)" | xargs perl -wapi art/tools/migration/art-3.10-migration.pl

use strict;

use vars qw(%header_list);

BEGIN {
    %header_list = (
        "canvas/Persistency/Provenance/ProvenanceFwd.h" => "canvas/Persistency/Provenance/fwd.h",
        "art/Framework/Core/Frameworkfwd.h" => "art/Framework/Core/fwd.h",
        );
}

foreach my $inc (sort keys %header_list) {
  s&^(\s*#include\s+["<])\Q$inc\E(.*)&${1}$header_list{$inc}${2}& and last;
}
