#!/usr/bin/env perl

use strict;
use warnings;

##
## Never use short namespace names, variable names, types!
## Perl and opencv have conflict symbols, such as 'cv',
## so we need have custom opencv, with renamed types
##
## /usr/lib64/perl5/CORE/sv.h:136:8: error: ‘struct cv’ redeclared as different kind of symbol
## struct cv {
##        ^
## In file included from /usr/include/opencv2/objdetect/objdetect.hpp:46:0,
##                 from src/ui/video.cpp:1:
## /usr/include/opencv2/core/core.hpp:70:14: error: previous declaration of ‘namespace cv { }’
##
## 20140211 morik@
##

sub my_rename
{
    my $code  = shift;
    my $files = shift;
    foreach my $file (@$files){
        my $content = '';

        ## load
        open IN, "<$file"
            or die "Cannot open file: '$file' for read ($!)";
        {
            local $/ = undef;
            $content = <IN>;
        }
        close IN;

        ## rename
        $content = $code->($content);

        ## save
        open OUT, ">$file.tmp"
            or die "Cannot open file: '$file.tmp' for write ($!)";
        print OUT $content;
        close OUT;

        print "rename '$file.tmp' to '$file'\n";

        rename "$file.tmp", "$file"
            or die ("Cannot rename '$file.tmp' to $file ($!)");
    }
}

my (@files, $res);
my $OPENCV = "opencv-2.4.5";

system("rm -rf $OPENCV");
(!system("tar -xzf $OPENCV.tar.gz"))
    or die ("Cannot untar $OPENCV.tar.gz");
chdir($OPENCV)
    or die ("Cannot change directory to '$OPENCV'");

$res   = `grep -IrP "cv::" . | awk -F\\: '{print \$1}' | sort -u | grep -v rename.pl`;
@files = split(/\n/, $res);
my_rename(sub { my $ret = shift; $ret =~ s/cv::/cv2::/g; return $ret;}, \@files);

$res   = `grep -IrP "cv;" . | awk -F\\: '{print \$1}' | sort -u | grep -v rename.pl`;
@files = split(/\n/, $res);
my_rename(sub { my $ret = shift; $ret =~ s/cv;/cv2;/g; return $ret;}, \@files);

$res   = `grep -IrP "namespace cv[\\s{]" . | awk -F\\: '{print \$1}' | sort -u | grep -v rename.pl`;
@files = split(/\n/, $res);
my_rename(sub { my $ret = shift; $ret =~ s/namespace cv([\s{])/namespace cv2$1/g; return $ret;}, \@files);

$res   = `grep -IrP "namespace cv\\s+{" . | awk -F\\: '{print \$1}' | sort -u | grep -v rename.pl`;
@files = split(/\n/, $res);
my_rename(sub { my $ret = shift; $ret =~ s/namespace cv(\s+{)/namespace cv2$1/g; return $ret;}, \@files);

$res   = `grep -IrP "namespace cv\\s*\$" . | awk -F\\: '{print \$1}' | sort -u | grep -v rename.pl`;
@files = split(/\n/, $res);
my_rename(sub { my $ret = shift; $ret =~ s/namespace cv(\s*[\r\n]+)/namespace cv2$1/g; return $ret;}, \@files);

$res   = `grep -IrP "cv22" . | awk -F\\: '{print \$1}' | sort -u | grep -v rename.pl`;
@files = split(/\n/, $res);
my_rename(sub { my $ret = shift; $ret =~ s/cv22/cv2/g; return $ret;}, \@files);

system("cmake .");
system("make");

