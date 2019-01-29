use strict;

package CetSkelPlugins::Producer;

use CetSkel::FilterProducerCommon;

use vars qw(@ISA);

@ISA = qw(CetSkel::FilterProducerCommon);

eval "use CetSkelPlugins::art::PluginVersionInfo";
unless ($@) {
  push @ISA, "CetSkelPlugins::art::PluginVersionInfo";
}

sub new {
  my $self = shift;
  my $type = ref($self) || $self;
  return bless { }, $type;
}

sub type { return "producer"; }

sub source { return __FILE__; }

my $header_files = { LEGACY => "art/Framework/Core/EDProducer.h",
                     SHARED => "art/Framework/Core/SharedProducer.h",
                     REPLICATED => "art/Framework/Core/ReplicatedProducer.h" };

my $base_classes = { LEGACY => "art::EDProducer",
                     SHARED => "art::SharedProducer",
                     REPLICATED => "art::ReplicatedProducer" };

my $ctor_args = { LEGACY => [ "fhicl::ParameterSet const& p" ],
                  SHARED => [ "fhicl::ParameterSet const& p",
                              "art::ProcessingFrame const& frame" ],
                  REPLICATED => [ "fhicl::ParameterSet const& p",
                                  "art::ProcessingFrame const& frame" ] };

my $produces_comment = "Call appropriate produces<>() functions here.";
my $consumes_comment = "Call appropriate consumes<>() for any products to be retrieved by this module.";
my $ctor_comments = { LEGACY => [ $produces_comment, $consumes_comment ],
                      SHARED => [ $produces_comment, $consumes_comment,
                                  "Call serialize<art::InEvent>(...) or async<art::InEvent>(...)." ],
                      REPLICATED => [ $produces_comment, $consumes_comment ] };

my $base_initializers = { LEGACY => "EDProducer{p}",
                          SHARED => "SharedProducer{p}",
                          REPLICATED => "ReplicatedProducer{p, frame}" };

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

# sub defineMacro from FilterProducerCommon.pm.

# sub macrosInclude from FilterProducerCommon.pm.

sub optionalEntries {
  my $self = shift;
  my $flavor = $self->{flavor};
  return
    {
     beginJob =>
         "void beginJob("
         .module_fn_args($flavor)
         .") override",
     beginRun =>
         "void beginRun("
         .module_fn_args($flavor, "art::Run& r")
         .") override",
     beginSubRun =>
         "void beginSubRun("
         .module_fn_args($flavor, "art::SubRun& sr")
         .") override",
     endJob =>
         "void endJob("
         .module_fn_args($flavor)
         .") override",
     endRun =>
         "void endRun("
         .module_fn_args($flavor, "art::Run& r")
         .") override",
     endSubRun =>
         "void endSubRun("
         .module_fn_args($flavor, "art::SubRun& sr")
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

# sub pluginSuffix from FilterProducerCommon.pm.

sub processOptions {
  my ($self, $options, $plugin_options) = @_;
  $self->processBasicOptions($options, $plugin_options);
}

sub requiredEntries {
  my $self = shift;
  my $flavor = $self->{flavor};
  return
    {
     produce =>
         "void produce("
         .module_fn_args($flavor, "art::Event& e")
         .") override"
    };
}

1;
