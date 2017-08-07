#!/bin/env perl

use strict;
use warnings;

my $sum = 0;
my $i   = 0;

while (1 > $i){
    if (!$i){
        $i = 0.001;
    } else {
        $i *= 1.05;
    };
    $sum += $i;
    print "$i\n"
};

print "total: $sum\n"

