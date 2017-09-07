use strict;

package CetSkelPlugins::Output;

use CetSkel::AnalyzerCommon qw(:DEFAULT pluginSuffix macrosInclude defineMacro);

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

sub type { return "output"; }

sub source { return __FILE__; }

sub baseClasses {
  return
    [
     { header => "art/Framework/Core/OutputModule.h",
       class => "art::OutputModule",
       protection => "public" # Default.
     }
    ];
}

sub constructors {
  return [ {
            explicit => 1,
            args => [ "fhicl::ParameterSet const & p" ],
            initializers => [ "OutputModule(p)" ]
           } ];
}

# sub defineMacro from AnalyzerCommon.pm.

sub implHeaders {
  return [ '"art/Framework/Principal/EventPrincipal.h"',
           '"art/Framework/Core/FileBlock.h"',
           '"art/Framework/Principal/RunPrincipal.h"',
           '"art/Framework/Principal/SubRunPrincipal.h"',
           '"art/Framework/Services/System/FileCatalogMetadata.h"',
           '"fhiclcpp/ParameterSet.h"' ];
}

# sub macrosInclude from AnalyzerCommon.pm.

sub optionalEntries {
  return
    {
     beginJob => "void beginJob() override",
     beginRun => "void beginRun(art::RunPrincipal const & r) override",
     beginSubRun => "void beginSubRun(art::SubRunPrincipal const & sr) override",
     doOpenFile => "void doOpenFile() override",
     doWriteFileCatalogMetadata => "void doWriteFileCatalogMetadata(art::FileCatalogMetadata::collection_type const & md) override",
     endJob => "void endJob() override",
     endRun => "void endRun(art::RunPrincipal const & r) override",
     endSubRun => "void endSubRun(art::SubRunPrincipal const & sr) override",
     finishEndFile => "void finishEndFile() override",
     isFileOpen => "bool isFileOpen() const override",
     openFile => "void openFile(art::FileBlock const & fb) override",
     reconfigure => "void reconfigure(fhicl::ParameterSet const & p) override",
     respondToCloseInputFile => "void respondToCloseInputFile(art::FileBlock const & fb) override",
     respondToCloseOutputFiles => "void respondToCloseOutputFiles(art::FileBlock const & fb) override",
     respondToOpenInputFile => "void respondToOpenInputFile(art::FileBlock const & fb) override",
     respondToOpenOutputFiles => "void respondToOpenOutputFiles(art::FileBlock const & fb) override",
     shouldWeCloseFile => "bool shouldWeCloseFile() const override",
     startEndFile => "void startEndFile() override",
     #writeBranchMapper => "void writeBranchMapper() override",
     writeEventHistory => "void writeEventHistory() override",
     writeFileFormatVersion => "void writeFileFormatVersion() override",
     writeFileIdentifier => "void writeFileIdentifier() override",
     writeFileIndex => "void writeFileIndex() override",
     writeParameterSetRegistry => "void writeParameterSetRegistry() override",
     writeParentageRegistry => "void writeParentageRegistry() override",
     writeProcessConfigurationRegistry => "void writeProcessConfigurationRegistry() override",
     writeProcessHistoryRegistry => "void writeProcessHistoryRegistry() override",
     writeProductDependencies => "void writeProductDependencies() override",
     writeProductDescriptionRegistry => "void writeProductDescriptionRegistry() override"
    };
}

# sub pluginSuffix from AnalyzerCommon.pm.

sub requiredEntries {
  return
    {
     write => "void write(art::EventPrincipal & e) override",
     writeRun => "void writeRun(art::RunPrincipal & r) override",
     writeSubRun => "void writeSubRun(art::SubRunPrincipal & sr) override"
    };
}

1;
