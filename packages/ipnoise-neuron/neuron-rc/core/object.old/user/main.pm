#!/usr/bin/env perl

package core::object::user::main;

use strict;
use warnings;

use core::templates::perl::generic;

use core::object::object::main;
use base 'core::object::object::main';

use constant GROUP_NAME_ADMIN => 'admin';

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

sub isAdmin
{
    my $self = shift;

    my $ret = 0;

    if ("core.users.god" eq $self->getId()){
        $ret = 1;
        goto out;
    }

    my $groups = $self->getNeighsByType(
        "core.object.param.group"
    );
    foreach my $group (@$groups){
        if (    "group" eq $group->getName()
            &&  GROUP_NAME_ADMIN eq $group->getValue())
        {
            $ret = 1;
            goto out;
        }
    }

out:
    return $ret;
}

sub getBookmarksObject
{
    my $self = shift;

    return $self->getNeighByType('core.object.bookmarks');
}

sub getNavName
{
    my $self = shift;

    my $ret     = 'незареганый пользователь';
    my $login   = $self->getParamByName('login');
    if ($login){
        $ret = 'пользователь '.$login->getValue();
    }

    return $ret;
}

sub getBasket
{
    my $self = shift;
    my $ret  =  $self->getNeighByType('core.object.basket');

    return $ret;
}

sub getLogin
{
    my $self    = shift;
    my $param   = $self->getParamByName('login');
    my $ret     = undef;

    $ret = $param->getValue();

    return $ret;
}

sub getPassword
{
    my $self    = shift;
    my $param   = $self->getParamByName('password');
    my $ret     = undef;

    $ret = $param->getValue();

    return $ret;
}

1;

