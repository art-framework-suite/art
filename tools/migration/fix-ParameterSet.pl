use strict;
use vars qw(%translations);

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
      }

foreach my $inc (sort keys %translations) {
  while (s&get\Q$inc\E\b&get$translations{$inc}&g) {};
  while (s&add\Q$inc\E\b&put$translations{$inc}&g) {};
}

### Local Variables:
### mode: cperl
### End:
