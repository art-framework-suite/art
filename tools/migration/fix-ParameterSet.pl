use strict;
use vars qw(%translations %inc_translations);

BEGIN { %translations = (
                         Bool => "<bool>",
                         FileInPath => "<FileInPath>",
                         Int => "<int>",
                         PSet => "<fhicl::ParameterSet>",
                         ParameterSet => "<fhicl::ParameterSet>",
                         String => "<std::string>",
                         UInt => "<unsigned int>",
                         Uint => "<unsigned int>",
                         VPSet => "<std::vector<fhicl::ParameterSet> >",
                         VString => "<std::vector<std::string> >",
                         VUInt => "<std::vector<unsigned int> >",
                         VUint => "<std::vector<unsigned int> >",
                        );
        %inc_translations = (
                             "art/ParameterSet/ParameterSet.h" => "fhiclcpp/ParameterSet.h",
                            );
      }

foreach my $name (sort keys %translations) {
  while (s&get\Q$name\E\b&get$translations{$name}&g) {};
  while (s&add\Q$name\E\b&put$translations{$name}&g) {};
}

foreach my $inc (sort keys %inc_translations) {
  s&^(\s*#include\s+["<])\Q$inc\E([">].*)$&${1}$inc_translations{$inc}${2}& and last;
}

### Local Variables:
### mode: cperl
### End:
