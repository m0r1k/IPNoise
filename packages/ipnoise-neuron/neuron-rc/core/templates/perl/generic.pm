#!/usr/bin/env perl

package core::templates::perl::generic;

use strict;
use warnings;

use core::object::object::main;

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
    my $args        = shift;
    my $object_id   = $args->{id};
    my $prefix      = $args->{prefix};
    my $view        = $args->{view};
    my $html        = '';

    my $context_old = $CONTEXT;

    warn("render:\n"
        ."  object_id: '$object_id'\n"
        ."  prefix:    '$prefix'\n"
        ."  view:      '$view'\n"
    );

    my $object = core::object::object::main::getObjectById(
        $object_id
    );

    $CONTEXT = {
        object_id   => $object_id,
        object      => $object,
        prefix      => $prefix,
        view        => $view
    };

    $html = $object->render($view);

    $CONTEXT = $context_old;

    return $html;
}

sub renderEmbedded
{
    my $args        = shift;
    my $object_id   = $args->{id};
    my $prefix      = $args->{prefix};
    my $view        = $args->{view};
    my $html        = '';

    my $context_old = $CONTEXT;

    warn("renderEmbedded:\n"
        ."  object_id: '$object_id'\n"
        ."  prefix:    '$prefix'\n"
        ."  view:      '$view'\n"
    );

    my $object = core::object::object::main::getObjectById(
        $object_id
    );

    $CONTEXT = {
        object_id   => $object_id,
        object      => $object,
        prefix      => $prefix,
        view        => $view
    };

    $html = $object->renderEmbedded($view);

    $CONTEXT = $context_old;

    return $html;
}

1;

