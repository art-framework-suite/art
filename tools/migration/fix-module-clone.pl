#!/usr/bin/perl -w

use strict;

use Cwd qw(chdir :DEFAULT);
use File::Basename;
use FileHandle;
use Getopt::Long;
use Pod::Usage;

Getopt::Long::Configure(qw(no_ignore_case bundling require_order));

my $prog = basename($0);
my $options = {};

GetOptions($options,
           "help|h|?",
           "force|f",
           ) or usage();

$options->{help} and usage();

my $module_name = $ARGV[0] || usage("No <class_name> specified.");
$module_name =~ s&.*::&&;
my $header_file = $ARGV[1] || usage("No <file_name> specified.");

if (system("grep -q -e '\bclone[ \t]*(?:\(|$)' \"$header_file\" >/dev/null 2>&1") == 0 and
    (!$options->{force})) {
  die "ERROR: file $header_file already appears to contain a definition of a clone()\nmethod -- specify -f to force.\n";
}

open(IN, "$header_file") or die "ERROR: Unable to open $header_file for read.\n";
my $outfile = `mktemp \${TMPDIR:-/tmp}/$prog.XXXXXXXXXX`;
chomp $outfile;
open(OUT, ">$outfile") or die "ERROR: Unable to open temporary file $outfile for output.\n";
my $in_class = 0;
my $public = 0;
my $clone_in = 0;
while (<IN>) {
  print;
  if (m&^\s*(class|struct)\s+(?:[\w\d_]+::)*\Q$module_name\E[^;]*$& and !$in_class) {
    $in_class = 1;
    $public = 1 if $1 eq "struct";
  } elsif (m&^\s*class\s+& and $in_class) {
    ++$in_class;
  }
  if (m&}\s*;& and $in_class and !$clone_in) {
    print OUT clone_method($public) unless --$in_class;
    $clone_in = 1;
  }
  $public = 1 if (m&^\s*public:\s*$& and $in_class == 1);
  if (m&^\s*private:& and $in_class == 1 and !$clone_in) {
    print OUT clone_method($public);
    $clone_in = 1;
  }
  print OUT;
}
close(IN);
close(OUT);

print "$outfile\n";
1;

sub usage {
  my $exitval = ($_[0] and $_[0] =~ m&^\d+$&)?shift:1;
  print STDERR "@ARGV\n" if scalar @ARGV;
  print STDERR join("", @_), "\n" if scalar(@_);
  print STDERR <<EOF;
usage: ${prog} --help|-h|-?
       ${prog} [--force|-f] <class_name> <file_name>
EOF
  exit($exitval);
}

sub clone_method {
  my $public = shift;
  my $result = $public?"":"public:\n";
  return "${result}  virtual $module_name * clone() const { return new $module_name(*this); }\n";
}

### Local Variables:
### mode: cperl
### End:
