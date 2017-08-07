#!/bin/env perl

use strict;
use warnings;
use Data::Dumper;

my $lines           = [];
my $dst_file_name   = undef;
my $src_file_name   = undef;

## get args
$dst_file_name = $ARGV[0];
if (!$dst_file_name){
    die("missing dst file name\n");
}
$src_file_name = $dst_file_name;
$src_file_name =~ s/(.*)\.tmp_(.+)$/$1.$2/;

print "src: $src_file_name => dst: $dst_file_name\n";

## read file
open IN, "<", "$src_file_name"
    or die("cannot open src file: '$src_file_name'"
        ." for dst_file_name: '$dst_file_name'\n");

while (<IN>){
    my $line = $_;
    chomp($line);

    my $line_number = scalar(@$lines) + 1;

    $line =~ s/FUNC_DEFINE/FUNC_DEFINE_$line_number/g;
    $line =~ s/FUNC_CALL/FUNC_CALL_$line_number/g;

    push @$lines, $line;
}

close IN;

## process lines
my $data = join("\n", @$lines);

## example:
##
## FUNC_DEFINE(
##    some_func       => int32_t,
##    const char *    => a_data,
##    const uint32_t  => a_data_size
## );
##
while ($data =~ /(FUNC_DEFINE_\d+)\s*\(([^)]+)\)/){
    my $id      = $1;
    my $info    = $2;

    $info =~ s/\n//g;
    my @parts = split(/,/, $info);

    my $func_name       = '';
    my $func_name_full  = '';
    my $return_type     = '';
    my $declarate       = '';
    my $args            = '';

    foreach my $part (@parts){
        my @key_val = split(/=>/, $part);
        if ($part eq $parts[0]){
            $func_name   = $key_val[0];
            $return_type = $key_val[1];

            $return_type =~ s/^\s+//g;
            $return_type =~ s/\s+$//g;
            $func_name   =~ s/^\s+//g;
            $func_name   =~ s/\s+$//g;

            $func_name_full = "$return_type $func_name";
        } else {
            my $arg_type = $key_val[0];
            my $arg_name = $key_val[1];

            $arg_type =~ s/^\s+//g;
            $arg_type =~ s/\s+$//g;

            $arg_name =~ s/^\s+//g;
            $arg_name =~ s/\s+$//g;

            if ($args){
                $args .= ", ";
            }

            $args .= "$arg_type $arg_name";
            $args =~ s/\s+/ /g;

            $func_name_full .= "_$arg_type";
            $func_name_full =~ s/\s+/ /g;
            $func_name_full =~ s/\*/ptr/g;
        }
    }

    ## remove spaces
    $func_name_full =~ s/\s/_/g;

    ## declarate func
    $declarate .= "$return_type $func_name_full ($args)";

    #print "args:        '$args'\n";
    #print "return_type: '$return_type'\n";
    #print "declarate:   '$declarate'\n";

    ## replace
    $data =~ s/$id\s*\([^)]+\)/$declarate/g;
}

## example:
##
## FUNC_CALL(
##    some_func       => int32_t,
##    const char *    => a_data,
##    const uint32_t  => a_data_size
## );
##
while ($data =~ /(FUNC_CALL_\d+)\s*\(([^)]+)\)/){
    my $id      = $1;
    my $info    = $2;

    $info =~ s/\n//g;
    my @parts = split(/,/, $info);

    my $func_name       = '';
    my $func_name_full  = '';
    my $return_type     = '';
    my $call            = '';
    my $args            = '';

    foreach my $part (@parts){
        my @key_val = split(/=>/, $part);
        if ($part eq $parts[0]){
            $func_name   = $key_val[0];
            $return_type = $key_val[1];

            $return_type =~ s/^\s+//g;
            $return_type =~ s/\s+$//g;
            $func_name   =~ s/^\s+//g;
            $func_name   =~ s/\s+$//g;

            $func_name_full = "$return_type $func_name";
        } else {
            my $arg_type = $key_val[0];
            my $arg_name = $key_val[1];

            $arg_type =~ s/^\s+//g;
            $arg_type =~ s/\s+$//g;

            $arg_name =~ s/^\s+//g;
            $arg_name =~ s/\s+$//g;

            if ($args){
                $args .= ", ";
            }

            $args .= "$arg_name";
            $args =~ s/\s+/ /g;

            $func_name_full .= "_$arg_type";
            $func_name_full =~ s/\s+/ /g;
            $func_name_full =~ s/\*/ptr/g;
        }
    }

    ## remove spaces
    $func_name_full =~ s/\s/_/g;

    ## call func
    $call .= "$func_name_full ($args)";

    #print "args:        '$args'\n";
    #print "return_type: '$return_type'\n";
    #print "call:   '$call'\n";

    ## replace
    $data =~ s/$id\s*\([^)]+\)/$call/g;
}

open OUT, ">", "$dst_file_name"
    or die("cannot open dst file name: '$dst_file_name'"
        ." for write\n");

print OUT "$data\n";
close OUT;

