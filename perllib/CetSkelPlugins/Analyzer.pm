use strict;

package CetSkelPlugins::Analyzer;

use CetSkel::AnalyzerCommon qw(:DEFAULT pluginSuffix macrosInclude defineMacro);

use vars qw(@ISA);

eval "use CetSkel::art::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkel::art::PluginVersionInfo";
}

sub new {
  my $class = shift;
  my $self = { };
  return bless \$self, $class;
}

sub type { return "analyzer"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => "art/Framework/Core/EDAnalyzer.h",
       class => "art::EDAnalyzer",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p" ],
            initializers => [ "EDAnalyzer(p)" ]
           } ];
}

# sub defineMacro from AnalyzerCommon.pm.

sub implHeaders {
  return $analyzer_headers; # From AnalyzerCommon.pm.
}

# sub macrosInclude from AnalyzerCommon.pm.

sub optionalEntries {
  return
    {
     beginJob => "void beginJob() override",
     beginRun => "void beginRun(art::Run const & r) override",
     beginSubRun => "void beginSubRun(art::SubRun const & sr) override",
     endJob => "void endJob() override",
     endRun => "void endRun(art::Run const & r) override",
     endSubRun => "void endSubRun(art::SubRun const & sr) override",
     reconfigure => "void reconfigure(fhicl::ParameterSet const & p) override",
     respondToCloseInputFile => "void respondToCloseInputFile(art::FileBlock const & fb) override",
     respondToCloseOutputFiles => "void respondToCloseOutputFiles(art::FileBlock const & fb) override",
     respondToOpenInputFile => "void respondToOpenInputFile(art::FileBlock const & fb) override",
     respondToOpenOutputFiles => "void respondToOpenOutputFiles(art::FileBlock const & fb) override"
    };
}

# sub pluginSuffix from AnalyzerCommon.pm.

sub requiredEntries {
  return
    {
     analyze => "void analyze(art::Event const & e) override"
    };
}

1;
