#!/usr/bin/env perl

use strict;
use warnings;

use core::templates::perl::generic;

my $html = '';

$html = processTemplate(
    'templates/neuron_type_4.tpl'
);

#print "html: '$html'\n";

