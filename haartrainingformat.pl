#!/usr/bin/perl
use strict;
use File::Basename;
################################################################################
# Convert a file format as follows:
#   File1 Left-x Upper-y Width Height
#   File1 Left-x Upper-y Width Height
#   File2 Left-x Upper-y Width Height
#   File2 Left-x Upper-y Width Height
#   File2 Left-x Upper-y Width Height
# to OpenCV haartraining suitable format as follows:
#   File1 2 Left-x Upper-y Width Height Left-x Upper-y Width Height
#   File2 3 Left-x Upper-y Width Height Left-x Upper-y Width Height Left-x Upper-y Width Height
#
# Usage:
#   perl haartrainingformat.pl [option]... [input]
# Options
#   -t
#   --trim
#     Trim heading 0 of x, y, width, height
#   -ls
#     convert a input format such as:
#       File1_Left-x_Upper-y_Width_Height.png
#       File2_Left-x_Upper-y_Width_Height.png
#
# Author: Naotoshi Seo
# Date  : 2008/08/28
################################################################################
my $infile;
my $lsformat = 0;
my $trim = 0; # trim heading 0 or not (0010 => 10)
my $basename = 0;
## arguments
for (my $i = 0; $i <= $#ARGV; $i++) {
    if ($ARGV[$i] eq "-t" || $ARGV[$i] eq "--trim" || $ARGV[$i] eq "-trim") {
        $trim = 1;
    } elsif ($ARGV[$i] eq "-b" || $ARGV[$i] eq "--basename" || $ARGV[$i] eq "-basename") {
        $basename = 1;
    } elsif ($ARGV[$i] eq "-ls" || $ARGV[$i] eq "--ls") {
    	$lsformat = 1;
    } elsif (substr($ARGV[$i],0,1) eq "-") {
        print "No such a option " . $ARGV[$i] . "\n";
        exit;
    } else {
        $infile = $ARGV[$i];
    }
}
## read
my @lines = ();
if (defined($infile)) {
    open(INPUT, $infile); @lines = <INPUT>; close(INPUT);
} else {
    @lines = <STDIN>;
}
## lsformat
if ($lsformat) {
    for (my $i = 0; $i <= $#lines; $i++) {
        if ($trim) {
            $lines[$i] =~ s/_0*(\d+)_0*(\d+)_0*(\d+)_0*(\d+)\.[^.]*$/ $1 $2 $3 $4\n/g;
        } else {
            $lines[$i] =~ s/_(\d+)_(\d+)_(\d+)_(\d+)\.[^.]*$/ $1 $2 $3 $4\n/g;
        }
    }
}

## body
my %counts = ();
my %coords = ();
foreach my $line (@lines) {
    $line =~ s/\s+$//;
    my @list = split(/ /, $line, 5);
    my $fname = shift(@list);
    if ($basename) { $fname = basename($fname); }
    if ($trim) {
        for (my $i = 0; $i <= $#list; $i++) {
            $list[$i] =~ s/^0*//;
        }
    }
    my $coord = ' ' . join(' ', @list);
    if (exists($counts{$fname})) {
        $counts{$fname}++;
        $coords{$fname} .= $coord;
    } else {
        $counts{$fname} = 1;
        $coords{$fname} = $coord;
    }
}
foreach my $fname (sort(keys(%counts))) {
    print $fname . ' ' . $counts{$fname} . $coords{$fname} . "\n";
}
