#!/usr/bin/env perl

package core::templates::perl::generic;

use strict;
use warnings;

use core::neuron::neuron::main;

our $log ||= core::templates::perl::log->new(
    copy_to_stderr => 1
);

our $CONTEXT = {};

use vars qw(
    @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS @SUBS
);

@ISA            = qw(Exporter);
@SUBS           = qw(
    render
    renderEmbedded
);
@EXPORT_OK      = @SUBS;
@EXPORT         = ();
%EXPORT_TAGS    = (
    ALL => [ @SUBS ]
);

sub render
{
    my $neuron_id   = shift;
    my $prefix      = shift;
    my $view        = shift;
    my $html        = '';

    my $context_old = $CONTEXT;

    warn("MORIK render neuron_id: '$neuron_id', view: '$view'\n");

    my $neuron = core::neuron::neuron::main::getNeuronById(
        $neuron_id
    );

    $CONTEXT = {
        neuron_id   => $neuron_id,
        neuron      => $neuron,
        prefix      => $prefix,
        view        => $view
    };

    $html = $neuron->render($view);

    $CONTEXT = $context_old;

    return $html;
}

sub renderEmbedded
{
    my $neuron_id   = shift;
    my $prefix      = shift;
    my $view        = shift;
    my $html        = '';

    my $context_old = $CONTEXT;

    warn("MORIK renderEmbedded neuron_id: '$neuron_id', view: '$view'\n");

    my $neuron = core::neuron::neuron::main::getNeuronById(
        $neuron_id
    );

    $CONTEXT = {
        neuron_id   => $neuron_id,
        neuron      => $neuron,
        prefix      => $prefix,
        view        => $view
    };

    $html = $neuron->renderEmbedded($view);

    $CONTEXT = $context_old;

    return $html;
}

1;

