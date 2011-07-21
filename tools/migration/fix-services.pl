use strict;

use vars qw(@optional @system);

BEGIN { @optional = qw(RandomNumberGenerator TFileService);
	@system = qw(CurrentModule FloatingPointControl TriggerNamesService);
    }

foreach my $service (@optional) {
  s&^(\s*#include\s+["<]art/Framework/Services/)Basic(/\Q${service}.h\E[">].*)$&${1}Optional${2}& and last;
}

foreach my $service (@system) {
  s&^(\s*#include\s+["<]art/Framework/Services/)Basic(/\Q${service}.h\E[">].*)$&${1}System${2}& and last;
}

s&^(\s*#include\s+["<]art/Framework/)Core/(RandomNumberGenerator)Service\.h&${1}Services/Optional/${2}.h&;

### Local Variables:
### mode: cperl
### End:
