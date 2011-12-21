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

my $header_file = $ARGV[0] || usage("No <file_name> specified.");
my $module_name = $ARGV[1] if $ARGV[1];
if ($module_name) {
  $module_name =~ s&.*::&&;
} else {
  $module_name = $header_file;
  $module_name =~ s&^(?:.*/)?(.*?)(?:_module)?\.(?:cxx|cc|C|cpp|h|hpp|H|hh)$&$1&;
  if ($module_name) {
    print "INFO: using deduced module class name $module_name.\n";
  } else {
    die "ERROR: Unable to deduce module class name from filename.\n";
  }
}

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
  if (m&^\s*(?:template.*)?(class|struct)\s+(?:[\w\d_]+::)*\Q$module_name\E[^;]*$& and !$in_class) {
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

my $result = 1;
if ($clone_in) {
  system("cp -p \"$header_file\" \"$header_file.~\" && mv \"$outfile\" \"$header_file\"")
    and die "ERROR: Unable to update header file from \"$outfile\": \"$outfile\" not deleted.\n";
  $result = 0;
} else {
  print STDERR <<EOF;
Unable to ascertain suitable place for insertion of clone method: insert
by hand in a public section of the module class definition (not necessary
for module template instantiations):

virtual $module_name * clone() const { return new $module_name(*this); }

EOF
}

print <<EOF;
Remeber to implement a non-trivial copy constructor if deep copy is
required for non-smart pointers or other resources.
EOF

exit($result);

sub usage {
  my $exitval = ($_[0] and $_[0] =~ m&^\d+$&)?shift:1;
  print STDERR "@ARGV\n" if scalar @ARGV;
  print STDERR join("", @_), "\n" if scalar(@_);
  print STDERR <<EOF;
usage: ${prog} --help|-h|-?
       ${prog} [--force|-f] <file_name> [<class_name>]
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
