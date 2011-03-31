use strict;

use vars qw(%inc_translations);

BEGIN {
  %inc_translations =
    ( "art/Framework/IO/(?:Input|Output)/" => "art/Framework/IO/Root/",
      "art/Version/GetFile" => "art/Framework/IO/Root/GetFile",
      "art/Framework/IO/GetFile" => "art/Framework/IO/Root/GetFile"
    );
}

foreach my $inc (sort keys %inc_translations) {
  s&^(\s*#include\s+["<])${inc}([^"]*[">].*)$&${1}$inc_translations{$inc}${2}& and last;
}

### Local Variables:
### mode: cperl
### End:
