use strict;

package CetSkelPlugins::Analyzer;

use CetSkel::AnalyzerCommon;

use vars qw(@ISA);

@ISA = qw(CetSkel::AnalyzerCommon);

eval "use CetSkelPlugins::art::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkelPlugins::art::PluginVersionInfo";
}

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  return bless { }, $type;
}

sub type { return "analyzer"; }

sub source { return __FILE__; }

my $header_files = { LEGACY => "art/Framework/Core/EDAnalyzer.h",
                     SHARED => "art/Framework/Core/SharedAnalyzer.h",
                     REPLICATED => "art/Framework/Core/ReplicatedAnalyzer.h" };

my $base_classes = { LEGACY => "art::EDAnalyzer",
                     SHARED => "art::SharedAnalyzer",
                     REPLICATED => "art::ReplicatedAnalyzer" };

my $ctor_args = { LEGACY => [ "fhicl::ParameterSet const& p" ],
                  SHARED => [ "fhicl::ParameterSet const& p",
                              "art::ProcessingFrame const& frame" ],
                  REPLICATED => [ "fhicl::ParameterSet const& p",
                                  "art::ProcessingFrame const& frame" ] };

my $consumes_comment = "Call appropriate consumes<>() for any products to be retrieved by this module.";
my $ctor_comments = { LEGACY => [ $consumes_comment ],
                      SHARED => [ $consumes_comment,
                                  "Call serialize<art::InEvent>(...) or async<art::InEvent>(...)." ],
                      REPLICATED => [ $consumes_comment ] };

my $base_initializers = { LEGACY => "EDAnalyzer{p}",
                          SHARED => "SharedAnalyzer{p}",
                          REPLICATED => "ReplicatedAnalyzer{p, frame}" };

sub module_fn_args {
  my $flavor = shift;
  my @args = @_;
  if ($flavor ne "LEGACY") {
      push @args, "art::ProcessingFrame const& frame";
  }
  return join(', ', @args);
}

sub baseClasses {
  my $self = shift;
  my $flavor = $self->{flavor};
  return
    [
     { header => $header_files->{$flavor},
       class => $base_classes->{$flavor},
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  my $self = shift;
  my $flavor = $self->{flavor};
  return [ { explicit => 1,
             args => $ctor_args->{$flavor},
             initializers => [ $base_initializers->{$flavor} ],
             comment => $ctor_comments->{$flavor}
           } ];
}

# sub defineMacro from AnalyzerCommon.pm.

# sub macrosInclude from AnalyzerCommon.pm.

sub optionalEntries {
  my $self = shift;
  if (! defined $self->{flavor}) {
      # If the flavor has not been defined, then that means the
      # 'help-type' program option has been provided to cetskelgen.  In
      # that case, we simply loop through the allowed flavors.
      my $all = {};
      foreach my $fl (qw(LEGACY SHARED REPLICATED)) {
          $all->{$fl} = "// ".$fl." flavor";
          my $entries_for_flavor = _optionalEntries($fl);
          foreach my $key (keys $entries_for_flavor) {
              $all->{$fl.$key} = delete $entries_for_flavor->{$key};
          }
      }
      return $all;
  }
  my $flavor = $self->{flavor};
  return _optionalEntries($flavor);
}

sub _optionalEntries {
  my $flavor = shift;
  return
  {
      beginJob =>
          "void beginJob("
          .module_fn_args($flavor)
          .") override",
      beginRun =>
          "void beginRun("
          .module_fn_args($flavor, "art::Run const& r")
          .") override",
      beginSubRun =>
          "void beginSubRun("
          .module_fn_args($flavor, "art::SubRun const& sr")
          .") override",
      endJob =>
          "void endJob("
          .module_fn_args($flavor)
          .") override",
      endRun =>
          "void endRun("
          .module_fn_args($flavor, "art::Run const& r")
          .") override",
      endSubRun =>
          "void endSubRun("
          .module_fn_args($flavor, "art::SubRun const& sr")
          .") override",
      respondToCloseInputFile =>
          "void respondToCloseInputFile("
          .module_fn_args($flavor, "art::FileBlock const& fb")
          .") override",
      respondToCloseOutputFiles =>
          "void respondToCloseOutputFiles("
          .module_fn_args($flavor, "art::FileBlock const& fb")
          .") override",
      respondToOpenInputFile =>
          "void respondToOpenInputFile("
          .module_fn_args($flavor, "art::FileBlock const& fb")
          .") override",
      respondToOpenOutputFiles =>
          "void respondToOpenOutputFiles("
          .module_fn_args($flavor, "art::FileBlock const& fb")
          .") override"
  };
}

# sub pluginSuffix from AnalyzerCommon.pm.

sub processOptions {
  my ($self, $options, $plugin_options) = @_;
  $self->processBasicOptions($options, $plugin_options);
}

sub requiredEntries {
  my $self = shift;
  if (! defined $self->{flavor}) {
      # If the flavor has not been defined, then that means the
      # 'help-type' program option has been provided to cetskelgen.  In
      # that case, we simply loop through the allowed flavors.
      my $all = {};
      foreach my $fl (qw(LEGACY SHARED REPLICATED)) {
          $all->{$fl} = "// ".$fl." flavor";
          my $entries_for_flavor = _requiredEntries($fl);
          foreach my $key (keys $entries_for_flavor) {
              $all->{$fl.$key} = delete $entries_for_flavor->{$key};
          }
      }
      return $all;
  }
  my $flavor = $self->{flavor};
  return _requiredEntries($flavor);
}

sub _requiredEntries {
  my $flavor = shift;
  return
  {
      analyze =>
          "void analyze("
          .module_fn_args($flavor, "art::Event const& e")
          .") override"
  };
}

1;
