use strict;

package CetSkelPlugins::ServiceInterface;

use CetSkel::ServiceCommon;

use vars qw(@ISA);

@ISA = qw(CetSkel::ServiceCommon);

eval "use CetSkel::art::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkel::art::PluginVersionInfo";
}

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  return bless { }, $type;
}

sub type { return "serviceinterface"; }

sub source { return __FILE__; }

# sub constructors not necessary.

# sub declHeaders from ServiceCommon.pm

sub declareMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DECLARE_ART_SERVICE_INTERFACE($qual_name, $self->{scope})
EOF
}

# sub defineMacro not necessary.

# sub fwdDeclarations not necessary.

# sub implHeaders not necessary.

# sub optionalEntries not necessary.

sub pluginSuffix {
  return ""; # No plugin suffix necessary, it's not technically a plugin.
}

sub processOptions {
  my ($self, $options, $plugin_options) = @_;
  $self->processBasicOptions($options, $plugin_options);
  $options->{split} = 1; # Enforced.
  $options->{generate} = { header => 1, plugin_source => 1 };
}

# sub requiredEntries not necessary.

1;
