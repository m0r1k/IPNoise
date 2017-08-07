#!/usr/bin/env perl

package core::templates::perl::Context;

use strict;
use warnings;

our $log ||= core::templates::perl::log->new(
    copy_to_stderr => 1
);

sub new
{
    my $invocant    = shift;
    my %args        = (
        session => {},
        user    => {},
        @_
    );

    my $class   = ref($invocant) || $invocant;
    my $user    = $args{user};
    my $session = $args{session};

    my $self = {
        %args,
        _log        => $log,
        _user       => core::object::user::main->new(
            props => $user
        ),
        _session    => core::object::session::main->new(
            props => $session
        )
    };

    return bless $self, $class;
}

sub getUser
{
    my $self = shift;
    my $ret  = $self->{_user};
    return $ret;
}

sub getSession
{
    my $self = shift;
    my $ret  = $self->{_session};
    return $ret;
}

1;

