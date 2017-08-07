#!/usr/bin/env perl

package core::templates::perl::log;

use strict;
use warnings;

use POSIX;
use Data::Dumper;
use Encode;
use Time::Local;

use constant COLOR_RED      => "\033[1;31m";
use constant COLOR_YELLOW   => "\033[1;33m";
use constant COLOR_GREEN    => "\033[0;32m";
use constant COLOR_GRAY     => "\033[0;39m";

use constant PRIVATE => __PACKAGE__
    .'_688c1982789733761926987687678687cda49515';

##
##  Don't use 'die' here! (it may call dead recursion)
##
##  20130312 rchechnev
##

##
##  new - constructor
##
sub new
{
    my $invocant    = shift;
    my %args        = (
        file            => undef,   ## log messages in file (default STDERR)
        debug_level     => 0,       ## minimum debug level for debug messages
        copy_to_stderr  => 0,       ## do copy log messages to STDERR
        color_support   => 1,
        @_
    );

    my $fd              = undef;
    my $err             = '';
    my $file            = $args{file};
    my $debug_level     = $args{debug_level};
    my $copy_to_stderr  = $args{copy_to_stderr};
    my $color_support   = $args{color_support};

    if ($file){
        ## try open file
        open $fd, ">>", $file
            or $err = "Cannot open file for write: '$file' ($!)";
    }

    if ($err){
        warn ("$err, all arguments below:\n".dumper(\%args));
        exit 1;
    }

    my $class    = ref($invocant) || $invocant;
    my $self     = {
        &PRIVATE => {
            file            => $file,
            fd              => $fd,
            copy_to_stderr  => $copy_to_stderr,
            debug_level     => $debug_level,
            color_support   => $color_support
        }
    };
    bless($self, $class);
    return $self;
}

## -------------------------- static methods ---------------------------------80

##
##  dump_stack - static method for show stack trace
##
sub dump_stack
{
    my $ret = '';
    my $i   = 0;

    my $stack    = [];
    my $info_old = undef;
    while (my @call_details = (caller($i++))){
        my $info = {
            'method'  => $call_details[3],
            'line'    => $info_old ? $info_old->{line} : '',
            'file'    => $info_old ? $info_old->{file} : ''
        };

        if ($info_old){
            push (@$stack, $info);
        }

        $info_old = {
            'what'    => $call_details[3],
            'method'  => $call_details[0],
            'line'    => $call_details[2],
            'file'    => $call_details[1]
        };
    }
    push (@$stack, $info_old);

    foreach my $call_info (@$stack){
        my $file    = $call_info->{file};
        my $method  = $call_info->{method};
        my $line    = $call_info->{line};
        my $what    = $call_info->{what};

        $ret .= sprintf "    %-50.50s (%s)\n",
            "$method:$line", $file;
    }

    return $ret;
}

##
##  dumper - static method for dump scalar
##
sub dumper
{
    my $scalar = shift;

    $Data::Dumper::Useqq = 1;
    no warnings 'redefine';
    sub Data::Dumper::qquote {
        my $s = shift;
        return "'$s'";
    }

    my $msg = Dumper($scalar);
    $msg =~ s/^\$VAR1\s*=\s*/dump: /;

    ## make dump pretty
    my @lines = split(/\n/, $msg);
    $msg = '';
    foreach my $line (@lines){
        $line =~ s/^\s{8}//;
        $msg .= "$line\n";
    }
    return $msg;
}

## -------------------------- generic methods --------------------------------80

sub debug
{
    my $self        = shift;
    my $debug_level = shift;
    my $msg         = shift;

    main::_pdebug($debug_level, $msg);
}

sub info
{
    my $self    = shift;
    my $msg     = shift;

    main::_pinfo($msg);
}

sub warn
{
    my $self    = shift;
    my $msg     = shift;

    main::_pwarn($msg);
}

sub error
{
    my $self    = shift;
    my $msg     = shift;

    main::_perror($msg);
}

sub fatal
{
    my $self    = shift;
    my $msg     = shift;

    chomp ($msg);
    $msg .= "\n";
    $msg .= "stack:\n";
    $msg .= dump_stack();
    $msg .= "dump:\n";
    $msg .= dumper(\@_);

    main::_pfatal($msg);
}

1;

