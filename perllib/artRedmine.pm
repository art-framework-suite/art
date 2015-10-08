use strict;

package artRedmine;

use Exporter;
use Redmine::API;
use vars qw(@ISA @EXPORT);

@ISA = qw(Exporter);
@EXPORT = qw(&artRedmine);

sub artRedmine {
  my $file = shift || glob("~/.redmine/.redmine_auth_key");
  open(KF, "$file") or die "Unable to open $file to read key";
  my $key = <KF>;
  chomp $key;
  return new Redmine::API(auth_key => $key, base_url => 'https://cdcvs.fnal.gov/redmine');
}

sub close_version {
  my ($api, $version_spec) = @_;
}

sub debug_print {
  my ($fh, @args) = @_;
  print $fh debug_print_one(\@args, 0), "\n";
}

sub debug_print_one {
  my ($item, $indent) = @_;
  if (not ref $item) {
    return sprintf('%s', $item);
  } elsif (ref $item eq "SCALAR") {
    return $$item;
  } elsif (ref $item eq "HASH") {
    return
      sprintf("{ %s\n%s}",
              join(sprintf(",\n%s", ' ' x ($indent + 2)),
                   map({ sprintf('%s => %s',
                                 $_,
                                 debug_print_one($item->{$_}, $indent + 2 + length("$_ => "))) }
                       sort keys %{$item})), ' ' x $indent);
  } elsif (ref $item eq "ARRAY") {
    return sprintf("[ %s\n%s]",
                   join(sprintf(",\n%s", ' ' x ($indent + 2)),
                        map { debug_print_one($_, $indent + 2) } @{$item}),
                  ' ' x $indent);
  } elsif (ref $item eq "JSON::PP::Boolean") {
    return debug_print_one($$item);
  } else {
    debug_print_one(ref $item, $indent);
  }
}

1;

### Local Variables:
### mode: cperl
### End:
