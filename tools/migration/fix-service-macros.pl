#!/usr/bin/perl -w

use strict;
use File::Basename;

my @services_cc = `find . -name '*_service.*' -print`;
chomp @services_cc;

my $tmp_file = `mktemp "\${TMPDIR:-/tmp}/fix-service-macros.XXXXXXXXXX"`;
chomp $tmp_file;
die "Unable to find temporary file name" if (!$tmp_file);

foreach my $service_cc (@services_cc) {
  next unless $service_cc =~ m&\.(?:C|cc|cpp|cxx)$&;
  print "Fixing service definition in ${service_cc} ... ";
  fix_macros($service_cc);
}

1;

sub fix_macros {
  my $service_cc = shift;
  my $header = find_header($service_cc);
  my $is_system_service = 0;
  my $declare_macro;
  open(SERVICE_CC, "<${service_cc}") or die "Unable to open ${service_cc} for read";
  open(TMPOUT, ">${tmp_file}") or die "Unable to open temporary file ${tmp_file} for write.";
  while (<SERVICE_CC>) {
    if (m&^\s*DECLARE_ART_(?:SERVICE|SYSTEM)&) {
      print "(already done).\n";
     # Already done.
      close(TMPOUT);
      return;
    }
    my ($service_spec, $args) = m&^\s*DEFINE_ART_((?:[^\s\(])+)\s*\((.*)\)& or
      do { print TMPOUT; next; };
    chomp;
    $is_system_service = ($service_spec eq "SYSTEM_SERVICE");
    $declare_macro = "DECLARE_ART_${service_spec}($args, LEGACY)";
    if (!$header) { # Put declare in .cc file for lack of anywhere else.
      print "(no header) ... ";
      print TMPOUT <<EOF;
// The DECLARE macro call should be moved to the header file, should you
// create one.
${declare_macro}
EOF
    }
    next if ($is_system_service); # No creator definition.
    print TMPOUT "$_\n";
  }
  close(SERVICE_CC);
  close(TMPOUT);
  system("mv \"${tmp_file}\" \"${service_cc}\"") == 0 or
    die "Unable to move edited ${service_cc} into place from ${tmp_file}";
  print "DONE.\n";
  insert_declare_in_header($header, $declare_macro) if ${header};
}

sub find_header {
  my $service_cc = shift;
  my ($header_stem) = (${service_cc} =~ m&^(.*)_service\.(?:C|cc|cpp|cxx)$&);
  my $dir =  dirname($header_stem);
  $header_stem = basename($header_stem);
  foreach my $ext qw(h H hh hpp hxx) {
    foreach my $test_h ("${dir}/${header_stem}.${ext}",
                        sprintf("%s/inc/${header_stem}.${ext}", dirname($dir)),
                        sprintf("%s/include/${header_stem}.${ext}", dirname($dir))) {
      if (-f "${test_h}") {
        return ${test_h};
      }
    }
  }
  return;
}

sub insert_declare_in_header {
  my ($header, $declare_macro) = @_;
  print "Fixing service header ${header} ... ";
  open(HEADER, "<${header}") or die "Unable to open ${header} for read";
  open(TMPOUT, ">${tmp_file}") or die "Unable to open temporary file ${tmp_file} for write.";
  my @header_content = <HEADER>; # Slurp.
  close(HEADER);
  if (grep m&^\s*DECLARE_ART_(?:SERVICE|SYSTEM)&, @header_content) {
    print "(already done).\n";
    return; # Already done.
  }
  my $counter = 0;
  foreach my $line (reverse @header_content) {
    --$counter; # Yes.
    last if $line =~ m&^\s*#\s*endif&;
  }
  splice @header_content, $counter, 0, ("${declare_macro}\n");
  print TMPOUT @header_content;
  close(TMPOUT);
  system("mv \"${tmp_file}\" \"${header}\"") == 0 or
    die "Unable to move edited ${header} into place from ${tmp_file}";
  print "DONE.\n";
}

__END__

### Local Variables:
### mode: cperl
### End:
