use strict;

package CetSkel::AnalyzerCommon;

use Getopt::Long qw(GetOptionsFromArray);
Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

my $flavors = { LEGACY => 1, SHARED => 1, REPLICATED => 1 };

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  return bless { }, $type;
}

# These will appear in the plugin interface of "subclasses" if not
# overridden therein.
sub declHeaders {
  return [ '"art/Framework/Principal/Event.h"',
           '"art/Framework/Principal/Handle.h"',
           '"art/Framework/Principal/Run.h"',
           '"art/Framework/Principal/SubRun.h"',
           '"canvas/Utilities/InputTag.h"',
           '"fhiclcpp/ParameterSet.h"',
           '"messagefacility/MessageLogger/MessageLogger.h"'
         ];
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_MODULE(${qual_name})
EOF
}

sub macrosInclude {
  return "art/Framework/Core/ModuleMacros.h";
}

sub pluginSuffix {
  return "_module";
}

sub usage {
  my $self = shift;
  return $self->basicUsage();
}

sub basicUsage {
  my $self = shift;
  my $type = $self->type();
  return <<EOF;
Usage: cetskelgen [options] [--] $type\[:<arg>] qualified-name
Args:
  [--flavor,<LEGACY|SHARED|REPLICATED>]
    Define the threading flavor of the analyzer (default LEGACY).
EOF
}

sub processBasicOptions {
  my ($self, $options, $plugin_options) = @_;
  GetOptionsFromArray($plugin_options,
                      "flavor=s" => sub { $self->_setFlavor(@_); }) or
                        do { usage(); exit(1); };
  $self->{flavor} = $self->{flavor} ? (uc $self->{flavor}) : "LEGACY";
}

sub _setFlavor {
  my ($self, $name, $flavor) = @_;
  $flavor = uc $flavor;
  if (not $flavors->{$flavor}) {
    die "ERROR: Illegal value for ", ref($self), "::$name option: $flavor.\n";
  }
  $self->{flavor} = $flavor;
}

1;

### Local Variables:
### mode: cperl
### End:
