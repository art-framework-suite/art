use strict;

package CetSkel::FilterProducerCommon;

use Exporter;

use vars qw(@ISA @EXPORT @EXPORT_OK);
use vars qw($fp_headers $fp_constructor_comment);
@ISA = qw(Exporter);
@EXPORT = qw($fp_headers $fp_constructor_comment);
@EXPORT_OK = qw(&pluginSuffix &macrosInclude &defineMacro); # From AnalyzerCommon, below.

use CetSkel::AnalyzerCommon qw(:DEFAULT pluginSuffix macrosInclude defineMacro);

$fp_headers = [ @$analyzer_headers, '<memory>' ];

$fp_constructor_comment = <<EOF;
Call appropriate produces<>() functions here.
EOF

1;

### Local Variables:
### mode: cperl
### End:
