use strict;

package CetSkelPlugins::Service;

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

sub type { return "service"; }

sub source { return __FILE__; }

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const& p", "art::ActivityRegistry& areg" ],
           } ];
}

# sub declHeaders from ServiceCommon.pm

sub declareMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DECLARE_ART_SERVICE($qual_name, $self->{scope})
EOF
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_SERVICE($qual_name)
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

# sub optionalEntries not necessary.

# sub pluginSuffix from ServiceCommon.pm.

sub processOptions {
  my ($self, $options, $plugin_options) = @_;
  $self->processBasicOptions($options, $plugin_options);
  if ($options->{split}) {
    $options->{generate} = { header => 1, plugin_source => 1 };
  }
}

# sub requiredEntries not necessary.

1;
