#!/usr/bin/perl

##
##  Roman E. Chechnev
##  Simple sys_select test script
##

use strict;
use warnings;

use Socket;
use IO::Select;
use Fcntl;
use Errno;
#use Data::Dumper;

use constant VERSION    => "0.02";
use constant PF_HOSTOS  => 36;

my $host = "127.0.0.1";
my $port = "55555";

if ($ARGV[0]){
    $port = $ARGV[0];
}

## create addr
my $iaddr = inet_aton($host);
my $paddr = sockaddr_in($port, $iaddr);

## create read set
my $read_set = new IO::Select();

my $fds = {};

sub add_fd
{
    my $fd      = shift;
    my $type    = shift;

    $fds->{fileno($fd)} = {
        fd      => $fd,
        type    => "$type",
        ipaddr  => "",
        port    => ""
    };

    fcntl($fd, F_SETFL, O_NONBLOCK)
        or die "fcntl: $!";

    binmode $fd;
    $read_set->add($fd);
}

sub close_fd
{
    my $fd = shift;

    send $fd, "bye!\n", 0;
    $read_set->remove($fd);

    print getname(fileno($fd))."> closed\n";
    delete $fds->{fileno($fd)};
    close($fd);
}

sub usage
{
    print <<HELP;
Usage:

HELP
}

sub getname
{
    my $fd  = shift;

    my $ret     = "";
    my $fd_info = $fds->{$fd};

    $ret = $fd.":".$fd_info->{"type"};
    return $ret;
}

## ------------- Main -------------

socket(TCP_SOCK, PF_HOSTOS, SOCK_STREAM, getprotobyname('tcp'))
    or die("Cannot create socket ($!)");

socket(UDP_SOCK, PF_HOSTOS, SOCK_DGRAM, getprotobyname('udp'))
    or die("Cannot create socket ($!)");


print fileno(TCP_SOCK)."> bind: $host:$port (tcp)\n";
print fileno(UDP_SOCK)."> bind: $host:$port (udp)\n";

bind TCP_SOCK, $paddr
    or die("Bind failed ($!)");

listen TCP_SOCK, 10
    or die("Listen failed ($!)");

bind UDP_SOCK, $paddr
    or die("Bind failed ($!)");

## add for watching
add_fd(\*TCP_SOCK, "tcp");
add_fd(\*UDP_SOCK, "udp");
add_fd(\*STDIN,    "stdin");

while (1){
    my $msg = "";
    foreach my $fd (keys %$fds){
        if ($msg){
            $msg .= ", ";
        }
        $msg .= getname($fd);
    }
    print "$msg> wait\n";
    my ($rh_set) = IO::Select->select(
        $read_set,
        undef,
        undef,
        120
    );
    foreach my $rh (@$rh_set) {
        if ($rh == \*TCP_SOCK) {
            my $ns = undef;
            my $res = accept $ns, $rh;
            if (defined $res){
                add_fd($ns, "tcp");
                print getname(fileno($ns))."> accepted\n";
                send $ns, getname(fileno($ns))."> hi there!\n", 0;
            }
        } elsif ($rh == \*UDP_SOCK) {
            my $buf = '';
            my $res = 0;
            $res = recv UDP_SOCK, $buf, 65535, 0;
            if (defined $res){
                chomp($buf);
                my($port, $ipaddr) = sockaddr_in($res);
                my $ip = gethostbyaddr($ipaddr, AF_INET);

                $fds->{fileno($rh)}->{"ipaddr"}   = $ipaddr;
                $fds->{fileno($rh)}->{"port"}     = $port;

                print getname(fileno($rh))." ($ip:$port) > $buf\n";
            }
        } elsif ($rh == \*STDIN) {
            my $line = <$rh>;
            if ($line =~ /help/){
                usage();
            }
        } else {
            my $buf = '';
            my $res = 0;
            $res = sysread $rh, $buf, 65535;
            if (defined $res){
                if ($res > 0){
                    chomp($buf);
                    print getname(fileno($rh))."> $buf\n";
                    if ($buf eq "quit"){
                        close_fd($rh);
                    }
                } elsif ($res == 0){
                    close_fd($rh);
                }
            }
        }
    }
}

