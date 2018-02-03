#!/usr/bin/perl

use Socket;
use Data::Dumper;
use Fcntl;
use POSIX;

use constant PF_HOSTOS => 44;

my $res;

sub nonblock
{
    my $fh = shift;
    my $flags = fcntl($fh, F_GETFL, 0)
        or die "Can't get flags for filehandle $fh: $!";
    fcntl($fh, F_SETFL, $flags | O_NONBLOCK)
        or die "Can't make filehandle $fh nonblocking: $!";
}

socket (my $sd, PF_HOSTOS, SOCK_STREAM, 0)
    or die "connect $!\n";

connect ($sd, pack_sockaddr_in(2525, inet_aton("localhost")))
    or die "connect $!\n";

## nonblock($sd);

my $errno;
my $data;
my $total_read = 0;

for(;;){
    my $buffer;

    $res = $sd->sysread($buffer, 1024*1024);
    $errno = POSIX::errno();

    ## print "read, buffer: '$buffer', res: '$res'\n";
    print "perl read, total_read: '$total_read', res: '$res', errno: '$errno'\n";

    if (not defined $res){
        if (EINTR == $errno){
            next;
        }
        if (EAGAIN == $errno){
            next;
        }
        last;
    } elsif (!$res) {
        last;
    }

    $data       .= $buffer;
    $total_read += $res;

    if ($buffer =~ /\n/){
        last;
    }
}

print "perl total_read: '$total_read'\n";

my $wrote = 0;
for (;;){
    print "perl write, already wrote: '$wrote'\n";
    $res = $sd->syswrite(
        substr($data, $wrote),
        $total_read - $wrote
    );
    $errno = POSIX::errno();
    print "perl write, res: '$res'\n";

    if (not defined $res){
        if (EINTR == $errno){
            next;
        }
        if (EAGAIN == $errno){
            next;
        }
        last;
    } elsif (!$res) {
        last;
    }

    $wrote += $res;
}

close $sd;

