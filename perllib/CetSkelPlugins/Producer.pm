use strict;

package CetSkelPlugins::Producer;

use CetSkel::FilterProducerCommon qw(:DEFAULT pluginSuffix macrosInclude defineMacro);

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

sub type { return "producer"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => "art/Framework/Core/EDProducer.h",
       class => "art::EDProducer",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p" ],
            comment => $fp_constructor_comment # From FilterProducerCommon.pm
           } ];
}

# sub defineMacro from FilterProducerCommon.pm.

sub implHeaders {
  return $fp_headers; # From FilterProducerCommon.pm.
}

# sub macrosInclude from FilterProducerCommon.pm.

sub optionalEntries {
  return
    {
     beginJob => "void beginJob() override",
     beginRun => "void beginRun(art::Run & r) override",
     beginSubRun => "void beginSubRun(art::SubRun & sr) override",
     endJob => "void endJob() override",
     endRun => "void endRun(art::Run & r) override",
     endSubRun => "void endSubRun(art::SubRun & sr) override",
     reconfigure => "void reconfigure(fhicl::ParameterSet const & p) override",
     respondToCloseInputFile => "void respondToCloseInputFile(art::FileBlock const & fb) override",
     respondToCloseOutputFiles => "void respondToCloseOutputFiles(art::FileBlock const & fb) override",
     respondToOpenInputFile => "void respondToOpenInputFile(art::FileBlock const & fb) override",
     respondToOpenOutputFiles => "void respondToOpenOutputFiles(art::FileBlock const & fb) override"
    };
}

# sub pluginSuffix from FilterProducerCommon.pm.

sub requiredEntries {
  return
    {
     producer => "void produce(art::Event & e) override"
    };
}

1;
