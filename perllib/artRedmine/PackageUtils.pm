use strict;

package artRedmine::PackageUtils;

use Exporter;

use vars qw(@ISA @EXPORT);

@ISA = qw(Exporter);

@EXPORT = qw(&for_package &ensure_gitconfig &vpage &dot_version &prod_version &nosep_version
             &tag_date &previous_version &by_version);

sub for_package {
  my $package = shift;
  my $command = join(" ", @_);
  return "" unless $command =~ m&\S&;
  my $result = `ssh p-$package\@cdcvs.fnal.gov \"$command\" 2>/dev/null`;
  chomp $result;
  return $result;
}

sub ensure_gitconfig {
  my $package = shift;
  for_package($package,
              "git config --global user.name '$package admin'; git config --global user.email 'artists\@fnal.gov'");
}

sub vpage {
  my $version = dot_version(shift);
  return "version:\"$version\"";
}

sub dot_version {
  my $dot_version = shift;
  $dot_version =~ s&^[^\d]*([^_\.]+)[\._]([^_\.]+)[\._]([^_\.]+)(.*)$&$1.$2.$3$4&;
  return $dot_version;
}

sub prod_version {
  my $version = dot_version(shift);
  $version =~ s/\./_/g;
  return "v$version";
}

sub nosep_version {
  my $version = dot_version(shift);
  $version =~ s/\.//g;
  return $version;
}

sub tag_date {
  my ($package, $tag, $repo) = @_;
  $repo = $package unless $repo;
  my $tag_date = for_package("$package", "cd /cvs/projects/$repo; git log -1 --simplify-by-decoration --pretty=\"format:\%cd\" --date=short \"$tag\"");
  $tag_date =~ s&-&/&g;
  return $tag_date;
}

sub previous_version($\@) {
  my $current = dot_version(shift);
  my $vref = shift;
  @$vref = map { dot_version($_); } @$vref;
  my $index = 0;
  foreach my $test_version (@$vref) {
    if ($test_version eq $current) {
      return $index ? $$vref[$index - 1] : 'none';
    }
    ++$index;
  }
  return "not found";
}

sub by_version($$) {
  my ($a, $b) = @_;
  # Requires dot versions.
  my @a = split /\./, $a, 3;
  my @b = split /\./, $b, 3;
  my ($a_extra, $b_extra);
  ($a[2], $a_extra) = ($a[2] =~ m&^(\d+)(.*)$&);
  ($b[2], $b_extra) = ($b[2] =~ m&^(\d+)(.*)$&);
  return
    $a[0] <=> $b[0] ||
      $a[1] <=> $b[1] ||
        $a[2] <=> $b[2] ||
          compare_extra($a_extra, $b_extra);
}

# Sort order:
#
# * aNN (alpha)
# * bNN (beta)
# * pre
# * rcNN (release candidate)
# * <empty>
# * pNN (patch)
# * Anything else (in lexical order).
#
# A leading '-' is ignored for sorting purposes.
sub compare_extra {
  my ($a, $b) = @_;
  # Ignore leading - for sort.
  $a =~ s&^-&&o;
  $b =~ s&^-&&o;
  if ($a eq "pre") { $a = "${a}0" }
  if ($b eq "pre") { $b = "${b}0" }
  my @a = ($a =~ m&^([abp]|rc|pre)(\d+)$&o);
  @a = ('x', $a) unless (scalar @a or not $a);
  my @b = ($b =~ m&^([abp]|rc|pre)(\d+)$&o);
  @b = ('x', $b) unless (scalar @b or not $b);
  my $enA = extra_num($a[0]);
  my $enB = extra_num($b[0]);
  return
    $enA <=> $enB ||
      (($enA == 0 || $enA == 2) ? ($a[1] cmp $b[1]) : ($a[1] <=> $b[1]));
}

sub extra_num {
  my ($in) = @_;
  return 0 unless $in;
  if ($in eq "x") {
    return 2;
  } elsif ($in eq "p") {
    return 1;
  } elsif ($in eq "rc") {
    return -1;
  } elsif ($in eq "pre") {
    return -2;
  } elsif ($in eq "b") {
    return -3;
  } elsif ($in eq "a") {
    return -4;
  } else {
    print STDERR "INTERNAL ERROR: unrecognized extra version prefix: $in\n";
    exit(1);
  }
}


1;

### Local Variables:
### mode: cperl
### End:
