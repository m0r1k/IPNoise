#!/usr/bin/env perl

package core::object::session::main;

use strict;
use warnings;

use core::templates::perl::generic;

use core::object::object::main;
use base 'core::object::object::main';

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

sub isRegistered
{
    my $self            = shift;
    my $ret             = 0;
    my $user            = $self->getUser();
    my $login_found     = 0;
    my $password_found  = 0;

    if ($user){
        my $params = $user->getParams();
        foreach my $param (@$params){
            my $param_name = $param->getName();
            if ("login" eq $param_name){
                $login_found = 1;
            } elsif ("password" eq $param_name){
                $password_found = 1;
                next;
            }
        }
    }

    if ($login_found && $password_found){
        $ret = 1;
    }

    return $ret;
}

sub getUser
{
    my $self = shift;
    return core::object::object::main::getCurUser();
}

sub getNavName
{
    my $self = shift;
    return 'сессия';
}

1;

