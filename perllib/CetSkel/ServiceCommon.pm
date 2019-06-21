use strict;

package CetSkel::ServiceCommon;

use Getopt::Long qw(GetOptionsFromArray);
Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

my $scopes = { LEGACY => 1, SHARED => 1 };

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  return bless { }, $type;
}

# These will appear in the plugin interface of "subclasses" if not
# overridden therein.
sub canMoveOrCopy {
  return 1;
}

sub declHeaders {
  my ($self) = @_;
  return [ sprintf('"%s"', $self->macrosInclude()) ];
}

sub macrosInclude {
  return "art/Framework/Services/Registry/ServiceMacros.h";
}

sub pluginSuffix {
  return "_service";
}

sub usage {
  my $self = shift;
  return $self->basicUsage();
}

# Functions to be used by "subclasses" or privately rather than via the plugin
# interface.
sub basicUsage {
  my $self = shift;
  my $type = $self->type();
  return <<EOF;
Usage: $type\[:<arg>[,<arg>]+]
Args:
  [--scope,<LEGACY|SHARED>]
    Define the scope of the service for the DECLARE and DEFINE service
    macros as applicable (default LEGACY).
EOF
}

sub processBasicOptions {
  my ($self, $options, $plugin_options) = @_;
  GetOptionsFromArray($plugin_options,
                      "scope=s" => sub { $self->_setScope(@_); }) or
                        do { usage(); exit(1); };
  $self->{scope} = $self->{scope} ? (uc $self->{scope}) : "LEGACY";
}

sub _setScope {
  my ($self, $name, $scope) = @_;
  $scope = uc $scope;
  if (not $scopes->{$scope}) {
    die "ERROR: Illegal value for ", ref($self), "::$name option: $scope.\n";
  }
  $self->{scope} = $scope;
}

1;

### Local Variables:
### mode: cperl
### End:
