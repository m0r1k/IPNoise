#!/usr/bin/perl

## doc
## http://en.wikipedia.org/wiki/Geostationary_orbit
## https://ru.wikipedia.org/wiki/%D0%9A%D1%80%D1%83%D0%B3%D0%BE%D0%B2%D0%BE%D0%B5_%D0%B4%D0%B2%D0%B8%D0%B6%D0%B5%D0%BD%D0%B8%D0%B5
## https://ru.wikipedia.org/wiki/%D0%9A%D0%BE%D0%BD%D0%B4%D0%B5%D0%BD%D1%81%D0%B0%D1%82_%D0%91%D0%BE%D0%B7%D0%B5_%E2%80%94_%D0%AD%D0%B9%D0%BD%D1%88%D1%82%D0%B5%D0%B9%D0%BD%D0%B0
## http://fphysics.com/massa

use strict;
use warnings;

use constant GEO_ORBIT_HEIGHT    => 3.5786e7;
use constant GEO_SATELLITE_SPEED => 3.0746e3;
use constant EARTH_RADIUS        => 6366e3;
use constant EARTH_WEIGHT        => 5.9726e24;
use constant EARTH_TO_SUN_LEN    => 1.496e11;
use constant GRAVITAT_CONST      => 6.67384e-11;
use constant LIGHT_SPEED         => 2.99792458e8;
use constant ELECTRON_CHARGE     => -1,602176565e-19;
use constant ELECTRON_WEIGHT     => 9.10938291e-31;
use constant PROTON_CHARGE       => 1;
use constant PROTON_WEIGHT       => 1.672621777e-27;
use constant BOROVSKY_RADIUS     => 5.2917720859e-11;
use constant PI                  => 3.1415926535;

sub test_G
{
    ## this function will calculate
    ## the gravitational constant
    ## from next information:
    ## satellite flying at geostationary orbit
    ## with satellite speed

    ## flight radius
    my $r = &GEO_ORBIT_HEIGHT + &EARTH_RADIUS;

    ## flight speed
    my $v = &GEO_SATELLITE_SPEED;

    ## centripetal acceleration
    ## my $a = $v * $v / $r;

    ## satellite's weight
    ## my $m = 1000;

    ## centripetal force
    ## my $fc = $m*$a
    ## my $fc = $m*$v*$v / $r

    ## gravital force
    ## my $fg = $G * $m * &EARTH_WiGHT / ($r*$r)

    ## stable flight
    ## $fc = $fg
    ## $m*$v*$v / $r = $G*$m*&EARTH_WiGHT / ($r*$r) 
    ## $G*$m*&EARTH_WiGHT = $m*$v*$v*$r*$r / $r
    ## $v = sqrt($G*$m*&EARTH_WiGHT*$r / $m*$r*$r)
    ## $v = sqrt($G*&EARTH_WiGHT / $r)
    ## $G = $m*$v*$v*$r*$r / $r*$m*&EARTH_WiGHT
    my $v_cal   = sqrt(&GRAVITAT_CONST*&EARTH_WiGHT / $r);
    my $G       = $v*$v*$r/&EARTH_WiGHT;

    my $m2 = (&GEO_SATELLITE_SPEED * &GEO_SATELLITE_SPEED * (&EARTH_RADIUS + &GEO_ORBIT_HEIGHT)) / &GRAVITAT_CONST;

    print "G constant:                 '".&GRAVITAT_CONST."'\n";
    print "G calculated:               '$G'\n";
    print "satellite speed constant:   '$v'\n";
    print "satellite speed calculated: '$v_cal'\n";
    print "proton charge calculated:   '".&GRAVITAT_CONST*&PROTON_WEIGHT."'\n";
    print "proton charge constant:     '".&PROTON_CHARGE."'\n";
    print "m2:                         '".$m2."'\n";
}

sub test_electorn
{
    ## centripetal acceleration
    ## my $a = $v * $v / $r;

    ## electron's weight
    ## my $m = 1000;

    ## centripetal force
    ## my $fctr = $m*$a
    ## my $fctr = $m*$v*$v / $r

    ## Coulomb's force
    ## my $fcul = $q1*$q2 / ($r*$r)

    ## stable flight
    ## $fctr = $fcul
    ## $m1*$v*$v / $r = $q1*$q2 / ($r*$r)
    ## $m1*$v*$v      = $q1*$q2 / $r 
    ## $q1 = G*$m1
    #$ $m1 = $q1/G
    ## $v  = sqrt($q1*$q2   / ($r*$m1))
    ## $v  = sqrt(G*$q1*$q2 / ($r*$q1))
    #$ $v  = sqrt(G*$q2     / $r)

    my $v = sqrt(
        (&GRAVITAT_CONST * &PROTON_CHARGE) / &BOROVSKY_RADIUS
    );

    ## $v = (2 * &PI * $r) / $T
    my $T = (2 * &PI * &BOROVSKY_RADIUS) / $v;
    my $f = 1 / $T;

    print "electron speed:      '$v'\n"; 
    print "electron period:     '$T'\n"; 
    print "electron freq:       '$f'\n"; 
}

sub test_to_from
{
    my $i = 0;

    my $q1 = &ELECTRON_CHARGE;
    my $q2 = &ELECTRON_CHARGE;
    my $m1 = &ELECTRON_WEIGHT;
    my $m2 = &ELECTRON_WEIGHT;

    #for ($i = 0.001; $i < 5000; $i += 1){
        my $r  = &EARTH_TO_SUN_LEN; ##$i * 2 * &BOROVSKY_RADIUS;
        my $f  = ($q1*$q2) / ($r*$r);
        my $a1 = ($q1*$q2) / ($r*$r*$m1);
        #my $a1 = $q2 / ($r*$r);
        my $t1 = sqrt((2*$r)/$a1);
        my $v  = $r / $t1;

        ## $m1*$a1 = ($q1*$q2) / ($r*$r);
        ## $r      = sqrt( ($q1*$q2) / ($m1*$a1) );
        ## $r      = 4*&PI*E0*

        print "i: '$i',"
            ." a1: '$a1',"
            ." t1: '$t1',"
            ." v:  '$v',"
            #." r: '$r',"
            #." f: '$f',"
            ."\n";
        
    #}
}

#print "-"x20,"\n";
#test_G();

#print "-"x20,"\n";
#test_electorn();

#print "-"x20,"\n";
#test_to_from();

## http://deviseblog.ru/3d-printer-nano/
## print 285e-6/&BOROVSKY_RADIUS,"\n";

## http://xage.ru/samyij-moschnyij-v-mire-mikroskop/
print 1e-9/&BOROVSKY_RADIUS,"\n";

