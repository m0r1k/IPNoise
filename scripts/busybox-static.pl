#!/usr/bin/perl

use strict;
use warnings;

sub usage
{
print <<END;
    Usage: $$ path_to_root_dir path_to_busy_box
END
}

my $path_to_root_dir = $ARGV[0];
my $path_to_busy_box = $ARGV[1];

unless ($path_to_root_dir){
    usage();
    exit 1;
}

unless ($path_to_busy_box){
    usage();
    exit 1;
}

my $out = `$path_to_root_dir/$path_to_busy_box 2>&1`;
$out =~ s|[\r\n,]+||g;
$out =~ s|.*functions:||g;

my @commands = split(/\s+/, $out);
foreach my $cmd (@commands){
    if (!$cmd || $cmd !~ /\w/){
        next;
    }
    system("ln -sf /$path_to_busy_box $path_to_root_dir/bin/$cmd");
}

