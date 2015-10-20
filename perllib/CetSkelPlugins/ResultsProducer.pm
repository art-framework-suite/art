use strict;

package CetSkelPlugins::ResultsProducer;

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

sub type { return "resultsproducer"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => 'art/Framework/Core/ResultsProducer.h',
       class => "art::ResultsProducer",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p" ]
           } ];
}

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_RESULTS_PLUGIN(${qual_name})
EOF
}

sub implHeaders {
  return [ '"art/Framework/Principal/Event.h"',
           '"art/Framework/Principal/Results.h"',
           '"art/Framework/Principal/Run.h"',
           '"art/Framework/Principal/SubRun.h"',
           '"fhiclcpp/ParameterSet.h"',
           '"messagefacility/MessageLogger/MessageLogger.h"' ];
}

# No sub macrosInclude necessary.

sub optionalEntries {
  return
    {
     beginJob => 'void beginJob() override',
     endJob => 'void endJob() override',
     beginSubRun => 'void beginSubRun(art::SubRun const & sr) override',
     endSubRun => 'void endSubRun(art::SubRun const &) override',
     beginRun => 'void beginRun(art::Run const & r) override',
     endRun => 'void endRun(art::Run const & r) override',
     event => 'void event(art::Event const & e) override',
     readResults => 'void readResults(art::Results const & res) override'
    };
}

# sub pluginSuffix from BasicPluginCommon.pm.

sub requiredEntries {
  return
    {
     writeResults => 'void writeResults(art::Results & res) override',
     clear => 'void clear() override'
    };
}

1;
