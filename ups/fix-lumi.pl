use strict;
use vars qw(%used_translations);
BEGIN { %used_translations = ();
      }

my ($from, $to) = ();

while (m/\G.*?([\w_\d]*lumi[\w_\d]*(?:\s*(?:block|section))?)/gi) {
  my $from = $1;
  $from =~ m/lumin[^o]/i and next; # eg Illuminate
  # Can run in identifier-only mode.
  $ENV{FIX_LUMI_CLASSES_ONLY} and $from !~ m&^[A-Z]& and next;
  my $to = $from;
  $to =~ s/lumi/subRun/;
  $to =~ s/Lumi/SubRun/;
  $to =~ s/(subrun)(?:nosity)?(?:\s*(?:block|section))?/${1}/i;
  if (exists $used_translations{$to}) {
    if ($used_translations{$to} ne $from) {
      # Oops
      print STDERR "FATAL: \"$from\" and \"$used_translations{$to}\" both translate to \"$to\" at line ",
        $., " in ", $ARGV, ".\n";
      exit(1);
    }
  } else {
    $used_translations{$to} = $from;
  }
}

for my $to (sort by_length_desc keys %used_translations) {
  my $from = $used_translations{$to};
  print STDERR "Fixing \"$from\" to \"$to\" at line ",
    $., " in ", $ARGV, ".\n";
  my @matches = s/\Q$from\E/\Q$to\E/g;
}
close ARGV if eof; # Reset $.

sub by_length_desc {
  return length($b) <=> length($a);
}

### Local Variables:
### mode: cperl
### End:
