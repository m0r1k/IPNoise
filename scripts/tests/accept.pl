#!/usr/bin/perl

use Socket;
use constant PF_HOSTOS => 36;

use strict;
use warnings;

my $host = "127.0.0.1";
my $port = "55555";

if ($ARGV[0]){
    $port = $ARGV[0];
}

socket(SOCK, PF_HOSTOS, SOCK_STREAM, getprotobyname('tcp'));

my $iaddr = inet_aton($host);
my $paddr = sockaddr_in($port, $iaddr);

print "bind: $host:$port\n";

bind SOCK, $paddr
    or die("Bind failed ($!)");

listen SOCK, 10
    or die("Listen failed ($!)");

for (;;){
    accept NEW_SOCK, SOCK
        or die("Accept failed ($!)");

    send NEW_SOCK, "hi there! my pid: '$$', please enter text:\n", 0
        or die("Send failed ($!)");

    my $answer = <NEW_SOCK>;
    chomp $answer;

    send NEW_SOCK, "you are typed: '$answer'\n", 0
        or die("Send failed ($!)");

    send NEW_SOCK, "bye!\n", 0
        or die("Send failed ($!)");

    close NEW_SOCK;
}

close SOCK;

