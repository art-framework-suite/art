use strict;

package CetSkelPlugins::EmptyEventTimestampPlugin;

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

sub type { return "emptyeventtimestampplugin"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => 'art/Framework/Core/EmptyEventTimestampPlugin.h',
       class => "art::EmptyEventTimestampPlugin",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const& p" ],
            initializers => [ "EmptyEventTimestampPlugin(p)" ]
           } ];
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_EMPTYEVENTTIMESTAMP_PLUGIN(${qual_name})
EOF
}

sub implHeaders {
  return [ '"art/Framework/Principal/Event.h"',
           '"art/Framework/Principal/Run.h"',
           '"art/Framework/Principal/SubRun.h"',
           '"art/Persistency/Provenance/Timestamp.h"',
           '"fhiclcpp/ParameterSet.h"',
           '"messagefacility/MessageLogger/MessageLogger.h"' ];
}

# No sub macrosInclude necessary.

sub optionalEntries {
  return
    {
     beginJob => 'void beginJob() override',
     endJob => 'void endJob() override',
     beginRun => 'void beginRun(art::Run const& r) override',
     beginRunTimestamp => 'art::Timestamp beginRunTimestamp(art::RunID const& rid) override',
     beginSubRun => 'void beginSubRun(art::SubRun const& sr) override',
     beginSubRunTimestamp => 'art::Timestamp beginSubRunTimestamp(art::SubRunID const& srid) override',
    };
}

# sub pluginSuffix from BasicPluginCommon.pm.

sub requiredEntries {
  return
    {
     eventTimestamp => 'art::Timestamp eventTimestamp(art::EventID const& eid) override',
     rewind => 'void rewind() override'
    };
}

1;
