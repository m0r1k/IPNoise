#!/usr/bin/env perl

use strict;
use warnings;
use Socket;

use constant PF_HOSTOS => 44;

my $dst_ip   = inet_aton("localhost");
my $dst_port = 2525;
my $dst      = sockaddr_in($dst_port, $dst_ip);
my $proto    = getprotobyname("tcp");
my $res;

socket(SOCK, PF_HOSTOS, SOCK_STREAM, $proto)
    or die ("cannot open socket ($!)");

$res = connect (SOCK, $dst)
    or die ("cannot connect to $dst_ip:$dst_port ($!)");


while (<SOCK>){
    my $line = $_;
    chomp ($line);
    print "was read: '$line', send it back\n";
    print SOCK "$line"
}

close (SOCK)
    or die ("cannot close socket ($!)");

1;

