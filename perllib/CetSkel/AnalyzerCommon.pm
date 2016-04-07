use strict;

package CetSkel::AnalyzerCommon;

use Exporter;

use vars qw(@ISA @EXPORT @EXPORT_OK);
use vars qw($analyzer_headers);
@ISA = qw(Exporter);
@EXPORT = qw($analyzer_headers);
@EXPORT_OK = qw(&pluginSuffix &macrosInclude &defineMacro);

$analyzer_headers =
  [ '"art/Framework/Principal/Event.h"',
    '"art/Framework/Principal/Handle.h"',
    '"art/Framework/Principal/Run.h"',
    '"art/Framework/Principal/SubRun.h"',
    '"canvas/Utilities/InputTag.h"',
    '"fhiclcpp/ParameterSet.h"',
    '"messagefacility/MessageLogger/MessageLogger.h"'
  ];

sub defineMacro {
  my ($self, $qual_name) = @_;
  return <<EOF;
DEFINE_ART_MODULE(${qual_name})
EOF
}

sub macrosInclude {
  return "art/Framework/Core/ModuleMacros.h";
}

sub pluginSuffix {
  return "_module";
}

1;

### Local Variables:
### mode: cperl
### End:
