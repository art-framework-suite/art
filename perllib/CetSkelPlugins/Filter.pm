use strict;

package CetSkelPlugins::Filter;

use CetSkel::FilterProducerCommon qw(:DEFAULT &pluginSuffix &macrosInclude &defineMacro);

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

sub type { return "filter"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => "art/Framework/Core/EDFilter.h",
       class => "art::EDFilter",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const& p" ],
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
     beginRun => "bool beginRun(art::Run& r) override",
     beginSubRun => "bool beginSubRun(art::SubRun& sr) override",
     endJob => "void endJob() override",
     endRun => "bool endRun(art::Run& r) override",
     endSubRun => "bool endSubRun(art::SubRun& sr) override",
     respondToCloseInputFile => "void respondToCloseInputFile(art::FileBlock const& fb) override",
     respondToCloseOutputFiles => "void respondToCloseOutputFiles(art::FileBlock const& fb) override",
     respondToOpenInputFile => "void respondToOpenInputFile(art::FileBlock const  &fb) override",
     respondToOpenOutputFiles => "void respondToOpenOutputFiles(art::FileBlock const& fb) override"
    };
}

# sub pluginSuffix from FilterProducerCommon.pm.

sub requiredEntries {
  return
    {
     filter => "bool filter(art::Event& e) override"
    };
}

1;
