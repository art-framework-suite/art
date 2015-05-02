use strict;

package CetSkelPlugins::ServiceImpl;

use CetSkel::ServiceCommon;

use vars qw(@ISA);

@ISA = qw(CetSkel::ServiceCommon);

eval "use CetSkelPlugins::art::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkelPlugins::art::PluginVersionInfo";
}

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  return bless { }, $type;
}

sub type { return "serviceimpl"; }

sub source { return __FILE__; }

sub baseClasses: {
  my $self = shift;
  return [ {
            header => $self->{interface_header},
            class => $self->{interface}
           } ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p", "art::ActivityRegistry & areg" ],
            comment => "Add interface initializer to list if necessary."
           } ];
}

# sub declHeaders from ServiceCommon.pm

sub declareMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DECLARE_ART_SERVICE_INTERFACE_IMPL($qual_name, $self->{interface}, $self->{scope})
EOF
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_SERVICE_INTERFACE_IMPL($qual_name, $self->{interface})
EOF
}

sub fwdDeclarations {
  my ($self, $indent) = @_;
  return <<EOF;
namespace fhicl {
${indent}class ParameterSet;
}

namespace art {
${indent}class ActivityRegistry;
}
EOF
}

sub implHeaders {
  return [ '"fhiclcpp/ParameterSet.h"',
           '"art/Framework/Services/Registry/ActivityRegistry.h"'
         ];
}

# sub implHeaders not necessary.

# sub optionalEntries not necessary.

# sub pluginSuffix from ServiceCommon.pm.

sub processOptions {
  my ($self, $options, $plugin_options) = @_;
  $options->{split} = 1; # Default.
  $self->processBasicOptions($options, $plugin_options);
  $options->{generate} = { header => 1, plugin_source => 1 };
  $self->{interface} = $$plugin_options[0] or
    do { print STDERR "ERROR: ", ref($self), " expected an interface to implement.\n",
           $self->usage(); exit(1); };
  $self->{interface_header} = $$plugin_options[1] || do
    {
      $_ = "$self->{interface}.h"; s&^.*::&&;
      print STDERR
        "WARNING: interface header not specified, using ",
          "\"$_\".\n";
      $_;
    }
}

# sub requiredEntries not necessary.

sub usage {
  my $self = shift;
  return sprintf("%s\n%s", $self->basicUsage(), <<EOF);
  <interface>[,<interface-header>]
    Specify the interface this service implements, and optionally the
    header include location (default "<interface>.h").
EOF
}

1;
