use strict;

package CetSkelPlugins::FileCatalogMetadataPlugin;

use CetSkel::BasicPluginCommon qw(:DEFAULT &pluginSuffix);

use vars qw(@ISA);

eval "use CetSkelPlugins::art::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkelPlugins::art::PluginVersionInfo";
}

sub new {
  my $class = shift;
  my $self = { };
  return bless \$self, $class;
}

sub type { return "filecatalogmetadataplugin"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => 'art/Framework/Core/FileCatalogMetadataPlugin.h',
       class => "art::FileCatalogMetadataPlugin",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p" ],
            initializers => [ "FileCatalogMetadataPlugin(p)" ]
           } ];
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_FILECATALOGMETADATA_PLUGIN(${qual_name})
EOF
}

# No sub implHeaders necessary.

# No sub macrosInclude necessary.

sub optionalEntries {
  return
    {
     beginJob => 'void beginJob() override',
     endJob => 'void endJob() override',
     collectMetadata => 'void collectMetadata(art::Event const & e) override',
     beginRun => 'void beginRun(art::Run const & r) override',
     endRun => 'void endRun(art::Run const & r) override',
     beginSubRun => 'void beginSubRun(art::SubRun const & sr) override',
     endSubRun => 'void endSubRun(art::SubRun const & sr) override'
    };
}

# sub pluginSuffix from BasicPluginCommon.pm.

sub requiredEntries {
  return
    {
     produceMetadata => 'auto produceMetadata() -> collection_type override'
    };
}

1;
