#!/usr/bin/env perl

package core::neuron::dialog::msg::main;

use strict;
use warnings;

use core::templates::perl::generic;

use core::neuron::neuron::main;
use base 'core::neuron::neuron::main';

use utf8;
use Encode;

##
## render
##
## >> view name
##
## << html
##
sub render
{
    my $self = shift;
    my $view = shift;
    my $ret  = '';

    my $tpl_path = $self->getTemplatePath(
        view        => $view,
        embedded    => 0,
        package     => __PACKAGE__
    );

    if (-r $tpl_path){
        $ret = $self->processTemplate($tpl_path);
    } else {
        ## trying SUPER
        $ret = $self->SUPER::render($view);
    }

    return $ret;
}

##
## renderEmbedded
##
## >> view name
##
## << html
##
sub renderEmbedded
{
    my $self = shift;
    my $view = shift;
    my $ret  = '';

    my $tpl_path = $self->getTemplatePath(
        view        => $view,
        embedded    => 1,
        package     => __PACKAGE__
    );

    if (-r $tpl_path){
        $ret = $self->processTemplate($tpl_path);
    } else {
        ## trying SUPER
        $ret = $self->SUPER::renderEmbedded($view);
    }

    return $ret;
}

sub getDate
{
    my $self = shift;
    my $ret  = '20131120 10:00';

    return $ret;
}

sub getUserId
{
    my $self = shift;
    my $ret  = 'some user id';

    return $ret;
}

sub getUserLogin
{
    my $self = shift;
    my $ret  = 'some user login';

    return $ret;
}

sub getText
{
    my $self = shift;
    my $ret  = $self->{props}->{text};

    return $ret;
}

1;

