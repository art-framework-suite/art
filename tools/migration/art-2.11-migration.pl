use strict;

use vars qw(%header_list);

BEGIN {
    %header_list = (
        "cetlib/coded_exception.h" => "cetlib_except/coded_exception.h",
        "cetlib/exception.h" => "cetlib_except/exception.h",
        "cetlib/exception_collector.h" => "cetlib_except/exception_collector.h"
        );
}

foreach my $inc (sort keys %header_list) {
  s&^(\s*#include\s+["<])\Q$inc\E(.*)&${1}$header_list{$inc}${2}& and last;
}
