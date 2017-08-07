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

sub log
{
    my $self    = shift;
    my $type    = uc(shift || 'FATAL');
    my $msg     = shift;
    chomp ($msg);

    my $color_support   = $self->{&PRIVATE}->{color_support};
    my $copy_to_stderr  = $self->{&PRIVATE}->{copy_to_stderr};
    my $dump_of_params  = '';
    my $date            = strftime("%Y%m%d %T", localtime);
    my $line            = sprintf "[ %-8.8s ] [ %-s ] [ %s ] %s\n",
        $type,
        $date,
        $$,
        $msg;

    if (scalar(@_)){
        $dump_of_params = dumper(\@_);
        $line .= "$dump_of_params";
    }

    Encode::_utf8_off($line);
    my $fd = $self->{&PRIVATE}->{fd};
    if ($fd){
        local *FH = $fd;
        print FH "$line";
    }

    ## force to stderr
    if (    'FATAL' eq $type
        ||  'ERROR' eq $type)
    {
        $copy_to_stderr = 1;
    }

    if ($copy_to_stderr){
        my $prefix = '';
        my $suffix = '';

        if ($color_support){
            if (    'ERROR' eq $type
                ||  'FATAL' eq $type)
            {
                $prefix = &COLOR_RED;
                $suffix = &COLOR_GRAY;
            } elsif ('WARN'    eq $type
                ||   'WARNING' eq $type)
            {
                $prefix = &COLOR_YELLOW;
                $suffix = &COLOR_GRAY;
            } elsif ('INFO' eq $type){
                $prefix = &COLOR_GREEN;
                $suffix = &COLOR_GRAY;
            }
        }

        print STDERR "$prefix$line$suffix";
    }

    if ('FATAL' eq $type){
        if ($fd){
            ## flush
            close($fd);
        }
        exit (1);
    }
}

sub debug
{
    my $self        = shift;
    my $debug_level = shift;
    my $msg         = shift;

    my $cur_debug_level = $self->{&PRIVATE}->{debug_level};
    if (    $cur_debug_level
        &&  $cur_debug_level >= $debug_level)
    {
        $self->log("DEBUG $debug_level", $msg, @_);
    }
}

sub info
{
    my $self    = shift;
    my $msg     = shift;

    $self->log('INFO', $msg, @_);
}

sub warn
{
    my $self    = shift;
    my $msg     = shift;

    $self->log('WARNING', $msg, @_);
}

sub error
{
    my $self    = shift;
    my $msg     = shift;

    $self->log('ERROR', $msg, @_);
}

sub fatal
{
    my $self    = shift;
    my $msg     = shift;

    chomp ($msg);
    $msg .= "\n";
    $msg .= "stack:\n";
    $msg .= dump_stack();

    $self->log('FATAL', $msg, @_);
}

sub parse_log_line
{
    my $line = shift;
    chomp ($line);

    my @parts;

    ## get all '[ some text ]' sections
    my @parts_tmp = $line =~ /(\s*\[\s*[^\]]+\]\s*)/g;
    foreach my $part (@parts_tmp){
        ## remove '[' and ']' symbols
        $part =~ s|[\[\]]||g;
        ## remove spaces from start and end
        $part =~ s|^\s+||;
        $part =~ s|\s+$||;
        push (@parts, $part);
    }

    my $type        = shift @parts;
    my $date        = shift @parts;
    my $pid         = shift @parts;
    my $date_year;
    my $date_month;
    my $date_day;
    my $date_hour;
    my $date_min;
    my $date_sec;
    my $unix_time;

    ## parse date if exist
    if ($date){
        if ($date =~ /^
            (\d{4})(\d{2})(\d{2})       ## 20130405
            \s+
            (\d{2})\:(\d{2})\:(\d{2})   ## 15:49:00
            $/x)
        {
            $date_year   = $1;
            $date_month  = $2;
            $date_day    = $3;
            $date_hour   = $4;
            $date_min    = $5;
            $date_sec    = $6;

            $unix_time = timelocal(
                $date_sec,
                $date_min,
                $date_hour,
                $date_day,
                $date_month,
                $date_year
            );
        }
    }

    my $ret = {
        type        => $type,
        date        => $date,
        date_year   => $date_year,
        date_month  => $date_month,
        date_day    => $date_day,
        date_hour   => $date_hour,
        date_min    => $date_min,
        date_sec    => $date_sec,
        pid         => $pid,
        unix_time   => $unix_time
    };

    return $ret;
}

1;

