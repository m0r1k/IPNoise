#!/usr/bin/perl

use Socket;

use strict;
use warnings;

use constant PF_HOSTOS => 36;

my $host = "localhost";
my $port = "2212";

socket(SOCK, PF_HOSTOS, SOCK_STREAM, getprotobyname('tcp'));

my $iaddr = inet_aton($host);
my $paddr = sockaddr_in($port, $iaddr);

connect(SOCK, $paddr)
    or die ("Connect failed\n");

send (SOCK, "hi there\n", 0)
    or die ("send failed($!)");

for (;;){
    my $answer = <SOCK>;
    chomp ($answer);
    print "was read: '$answer'\n";
    if ($answer eq "quit"){
        send SOCK, "Bye!", 0;
        close SOCK;
        last;
    }
    my $read = <>;
    send SOCK, "$read", 0;
}

