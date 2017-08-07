#!/usr/bin/env perl

package core::neuron::neuron::main;

use strict;
use warnings;

use utf8;
use Encode;
use Data::Dumper;

our $log ||= core::templates::perl::log->new(
    copy_to_stderr => 1
);

use core::templates::perl::log;
use core::templates::perl::generic;
use core::templates::perl::Context;

use constant TEMPLATES_DIR => "templates";

use vars qw(
    @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS
    @SUBS
);

@ISA  = qw(Exporter);
@SUBS = qw(
    createNeuron
    getCurNeighs
    getCurNeighsByType
    getCurNeuron
    getCurNeuronParams
    getContext
    getContextCpp
    getCurNeuronId
    getCurUser
    getDreamlandNeuron
    getNeuronById
    getGodNeuron
    getShopNeuron
    getSession
    getHref
    getNeuronInfo
    processTemplate
    processCurTemplate
    render
    renderEmbedded
);
@EXPORT_OK      = (@SUBS);
@EXPORT         = qw();
%EXPORT_TAGS    = (
    ALL => [ @SUBS ]
);

# preload neurons info
my $neurons_info = eval "main::_getNeuronsInfo();";

## constructor
sub new
{
    my $invocant    = shift;
    my %args        = (
        @_
    );

    my $class = ref($invocant) || $invocant;

    my $self = {
        %args,
        _log => $log
    };

    return bless $self, $class;
}

sub getLog
{
    my $self = shift;
    return $self->{_log};
}

##
## getId
##
## << Neuron ID
##
sub getId
{
    my $self = shift;
    my $ret  = $self->{props}->{_id};
    return $ret;
}

##
## getParams
##
## >> param 'name'
## << ARRAYREF with NeuronParams
##
sub getParams
{
    my $self = shift;
    my %args = (
        name    => undef,
        @_
    );

    my $ret  = [];
    my $res  = [];
    my $name = $args{name};

    $res = $self->getNeuronParams();
    foreach my $param (@$res){
        if (    $name
            &&  $name ne $param->getName())
        {
            next;
        }
        push (@$ret, $param);
    }
    return $ret;
}

##
## getParamsByName
##
## >> name
## << ARRAYREF with NeuronParams
##
sub getParamsByName
{
    my $self = shift;
    my $name = shift;

    my $params = $self->getParams(
        name => $name
    );

    return $params;
}

##
## getParamByName
##
## >> name
## << NeuronParam
##
sub getParamByName
{
    my $self = shift;
    my $name = shift;

    my $params = $self->getParamsByName($name);
    if (1 < scalar(@$params)){
        $log->pwarn(
            "more than 1 param found for name: '$name'"
        );
    }
    return $params->[0];
}

##
## getParamValue
##
## >> param name
## << param value
##
sub getParamValue
{
    my $self = shift;
    my $name = shift;

    my $ret     = undef;
    my $param   = $self->getParamByName($name);

    if ($param){
        $ret = $param->getValue();
    }

    return $ret;
}

##
## getType
##
## << type as string
##
sub getType
{
    my $self = shift;
    my $type = $self->{props}->{type};
    return $type;
}

sub getTemplatePath
{
    my $self = shift;
    my %args = (
        view        => undef,
        embedded    => 0,
        package     => undef,
        @_
    );

    my $log         = $self->getLog();
    my $view        = $args{view};
    my $embedded    = $args{embedded};
    my $package     = $args{package};
    my $cur_user    = $self->getCurUser();
    my $err         = '';
    my $path        = '';
    my $prefix      = getContext()->{prefix};

    if (!defined $view){
        if ($err){
            $err .= ', ';
        }
        $err .= " missing argument: 'view'";
    }
    if (!defined $embedded){
        if ($err){
            $err .= ', ';
        }
        $err .= " missing argument: 'embedded'";
    }
    if (!defined $package){
        if ($err){
            $err .= ', ';
        }
        $err .= " missing argument: 'package'";
    }
    if ($err){
        $log->fatal($err, {
            args => \%args
        });
    };

    my   @parts = split("::", $package);
    pop  @parts; ## remove 'main'
    push @parts, &TEMPLATES_DIR;
    push @parts, $prefix;

    my $templates_dir = join("/", @parts);

    if ($embedded){
        $path = "$templates_dir/$view.tpl";
    } elsif ($cur_user->isAdmin()){
        $path = "admin/index.tpl";
    } else {
        $path = "index.tpl";
    }

    return $path;
}

##
## render
##
## >> view name
##
## << html
##
sub render
{
    my $self = shift;
    my $view = shift;
    my $log  = $self->getLog();
    my $ret  = '';

    if (!defined $view){
        $log->pfatal("missing argument 'view'");
    }

    my $tpl_path = $self->getTemplatePath(
        view        => $view,
        embedded    => 0,
        package     => __PACKAGE__
    );

    $ret = $self->processTemplate(
        $tpl_path,
        $view
    );

    return $ret;
}

##
## renderEmbedded
##
## >> view name
##
## << html
##
sub renderEmbedded
{
    my $self = shift;
    my $view = shift;
    my $log  = $self->getLog();
    my $ret  = '';

    if (!defined $view){
        $log->pfatal("missing argument 'view'");
    }

    my $tpl_path = $self->getTemplatePath(
        view        => $view,
        embedded    => 1,
        package     => __PACKAGE__
    );

    warn ("MORIK self neuron_id: '".$self->getId()."' tpl_path: '$tpl_path'\n");

    $ret = $self->processTemplate($tpl_path);

    return $ret;
}

##
## getNeighs - return neuron neighs
##
## >> param 'type' (string or int) or ARRAYREF with string or int
##
## << array of Neigh objects
##
sub getNeighs
{
    my $self = shift;
    my %args = (
        type    => undef,
        types   => [],
        @_
    );

    my $ret     = [];
    my $types   = $args{types};
    my $type    = $args{type};

    if ('ARRAY' eq (ref $type)){
        push (@$types, @$type);
    } elsif (defined $type){
        push (@$types, $type);
    }

    foreach my $neigh_id (keys %{$self->{props}->{neighs}}){
        my $skip    = @$types;
        my $neigh   = getNeuronById($neigh_id);

        if (!$neigh){
            next;
        }

        foreach my $type (@$types){
            if (    defined $type
                &&  $neigh->getType() eq $type)
            {
                $skip = 0;
                last;
            }
        }
        if ($skip){
            next;
        }
        push (@$ret, $neigh);
    }

    return $ret;
}

##
## getNeighsByType - return neuron neighs by type
##
## >> type (string)
##
## << array of Neigh objects
##
sub getNeighsByType
{
    my $self    = shift;
    my $type    = shift;
    my $ret     = [];

    $ret = $self->getNeighs(
        type => $type
    );

    return $ret;
}

##
## getNeighByType - return neuron neigh by type
##
## >> type (string or int)
##
## << Neigh object
##
sub getNeighByType
{
    my $self    = shift;
    my $type    = shift;

    my $res = [];
    $res = $self->getNeighsByType($type);

    if (1 < scalar(@$res)){
        warn "more than 1 neigh found"
            ." for type: '$type',"
            ." neuron ID: '".$self->getId()."'";
    }

    return $res->[0];
}

##
## getNeuronParams
##
## >>
##
## << array with Param objects
##
sub getNeuronParams
{
    my $self    = shift;
    my $ret     = [];
    my $id      = $self->getId();
    my $params  = undef;

    if (!defined $id){
        $log->fatal("cannot get object ID\n", {
            self => $self
        });
    }

    $params = main::_getNeuronParams($id) || [];
    foreach my $cur_param (@$params){
        my $param = createNeuron(props => $cur_param);
        push (@$ret, $param);
    }

    return $ret;
}

sub getName
{
    my $self = shift;
    my $ret  = undef;

    $ret = $self->{props}->{name};

    return $ret;
}

sub getTitle
{
    my $self = shift;
    my $url  = $self->{props}->{title};

    return $url;
}

sub getUrl
{
    my $self = shift;
    my $url  = $self->{props}->{url};

    return $url;
}

##
## getNavName - get navigation name
##
## << name
##
sub getNavName
{
    my $self = shift;
    return $self->getId();
}

##
## getProps - get object properties
##
## << HASHREF with property name and value
##
sub getProps
{
    my $self = shift;
    my $ret  = {};

    $ret = $self->{props};

    return $ret;
}

##
## getHtmlAttrs - get object properties
##
## << string with attrs for HTML
##
sub getHtmlAttrs
{
    my $self    = shift;
    my $ret     = '';
    my $props   = $self->getProps();

    foreach my $prop_name (keys %$props){
        my $prop_val = $props->{$prop_name};
        if (    !defined $prop_val
            ||  ref $prop_val)
        {
            next;
        }
        if ('_id' eq $prop_name){
            $prop_name = 'neuron'.$prop_name;
        } else {
            $prop_name = 'neuron_'.$prop_name;
        }
        if ($ret){
            $ret .= ' ';
        }
        $ret .= $prop_name.'="'.$prop_val.'"';
    }

    return $ret;
}

## ---------------- static ----------------

sub createNeuron
{
    my %args    = (
        props   => undef,
        @_
    );

    my $neuron  = undef;
    my $type    = $args{props}->{type};

    foreach my $key (keys %{$args{props}}){
        Encode::_utf8_on($args{props}->{$key});
    }

    ## create package name
    my @tmp     = split(/\./, $type);
    my $package = join("::", @tmp) . "::main";

    ## load file via fake package
    eval "use $package;";
    if ($@){
        $log->fatal("cannot use package: '$package' ($@)\n");
    }

    eval {
        $neuron = $package->new(%args);
    };
    if ($@){
        $log->fatal("cannot create class '$package' ($@)\n");
    }

    return $neuron;
}

##
## getNeuronById
##
## >> neuron_id
##
## << Neuron object or undef
##
sub getNeuronById
{
    my $ret = undef;
    my $res = main::_getNeuronById(@_);
    if ($res && $res->{_id}){
        $ret = createNeuron(props => $res);
    }
    return $ret;
}

##
## getCurUser
##
## << User object or undef
##
sub getCurUser
{
    my $ret = undef;
    my $res = main::_getCurUser() || {};
    if ($res && $res->{_id}){
        $ret = createNeuron(props => $res);
    }
    return $ret;
}

##
## getCurNeuron
##
## << Neuron object or undef
##
sub getCurNeuron
{
    my $ret = undef;
    $ret = getContext()->{neuron};
    return $ret;
}

##
## getCurNeuronId
##
## << Neuron ID or undef
##
sub getCurNeuronId
{
    my $ret = getContext()->{neuron_id};
    return $ret;
}

###
## getGodNeuron
##
## << God Neuron
##
sub getGodNeuron
{
    my $neuron = getNeuronById('core.users.god');
    return $neuron;
}

##
## getShopNeuron
##
## << Shop Neuron
##
sub getShopNeuron
{
    my $neuron = getNeuronById('core.shop');
    return $neuron
}

##
## getDreamlandNeuron
##
## << Dreamland Neuron
##
sub getDreamlandNeuron
{
    my $neuron = getNeuronById('core.dreamland');
    return $neuron
}

##
## getCurNeuronParams
##
## << array with Param objects
##
sub getCurNeuronParams
{
    my $ret        = [];
    my $cur_neuron = getCurNeuron();
    if ($cur_neuron){
        $ret = $cur_neuron->getNeuronParams();
    }
    return $ret;
}

##
## getContextCpp
##
## << C++ context object
##
sub getContextCpp
{
    my $ret     = main::_getContext() || {};
    my $context = core::templates::perl::Context->new(%$ret);
    return $context;
}

##
## getContext
##
## << Context HASHREF
##
sub getContext
{
    my $ret = $core::templates::perl::generic::CONTEXT;
    return $ret;
}

##
## getSession
##
## << Session object or undef
##
sub getSession
{
    my $ret = undef;
    my $res = main::_getSession() || {};
    if ($res && $res->{_id}){
        $ret = createNeuron(props => $res);
    }

    return $ret;
}

##
## getNeuronInfo
##
## << HASHREF with neurons info
##
sub getNeuronsInfo
{
    my $ret = $neurons_info;
    Encode::_utf8_on($ret);
    return $ret;
}

## -------------------- log --------------------

sub dump_stack
{
    my $ret = '';
    my $i   = 0;

    my $stack = [];
    while (my @call_details = (caller($i++))){
        my $info = {
            'file'    => $call_details[0],
            'line'    => $call_details[2],
            'package' => $call_details[1]
        };
        push (@$stack, $info);
    }

    foreach my $call_info (@$stack){
        my $file    = $call_info->{file};
        my $line    = $call_info->{line};
        my $package = $call_info->{package};

        $ret .= '    ';
        $ret .= "$file:$line ($package)\n";
    }

    return $ret;
}

sub dumper
{
    my $scalar  = shift;
    my $msg     = Dumper($scalar);

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

## ---------------------------------------------

sub string_to_hash
{
    my $string = shift;
    my $hash   = {};

    if ($string){
        eval "$hash = $string;";
    }

    return $hash;
}

##
##  processCurTemplate - process template
##    in current template type directory
##
sub processCurTemplate
{
    my $self    = shift;
    my $fname   = shift;
    my $path    = '';

    ## for static calls
    if ('' eq ref $self){
        $fname  = $self;
        $self   = getCurNeuron();
    }

    if (!defined $fname){
        $log->fatal("missing argument: 'fname'");
    }

    ## TODO FIXME
    $fname =~ s/\.tpl$//g;
    my $html = $self->renderEmbedded($fname);
    return $html;
}

sub process_perl
{
    my $fname = shift;
    my $code  = shift;

    my $html = '';

    $log->warn("process perl code\n", {
        code => $code
    });

    $html = eval "use utf8;$code";
    if ($@){
        my $err = "Cannot parse template: '$fname' ($@)\n";
        $log->fatal($err, {
            fname   => $fname,
            code    => $code
        });
    }

    return $html;
}

##
##  processTemplate - process template in global template directory
##
sub processTemplate
{
    my $self    = shift;
    my $fname   = shift;
    my $view    = shift;
    my $prefix  = getContext()->{prefix};
    my $html    = '';
    my $res;

    ## for static calls
    if ('' eq ref $self){
        $fname  = $self;
        $self   = getCurNeuron();
    }

    warn "Neuron::processTemplate fname: '$fname' ".(ref $self)."\n";

    ## for static calls without neuron
    if ($fname !~ /^core\//){
        $fname = "core/templates/$prefix/$fname";
    }

    warn "MORIK fname: '$fname'";

    $html = "<!-- $fname { -->\n";

    {
        my $char        = '';
        my $outer_start = 0;
        my $outer_end   = 0;
        my $inner_start = 0;
        my $inner_end   = 0;

        ## open file
        open my $fd, '<:encoding(UTF-8)', $fname
            or  $log->fatal("Cannot open file: '$fname'"
                    ." for read ($!)");

        while (read $fd, $char, 1){
            my $start_text  = '<perl>';
            my $end_text    = '</perl>';
            my $pos         = -1;

            ## store html
            $html .= $char;

            ## search start
            $pos  = length($html) - length($start_text);
            if (    $pos >= 0
                &&  substr($html, $pos) eq $start_text)
            {
                $outer_start = $pos;
                $inner_start = $pos + length($start_text);
            }

            ## search end
            $pos  = length($html) - length($end_text);
            if (    $pos >= 0
                &&  substr($html, $pos) eq $end_text)
            {
                $outer_end = $pos + length($end_text);
                $inner_end = $pos;
                my $code = substr(
                    $html,
                    $inner_start,
                    $inner_end - $inner_start
                );
                $res = process_perl($fname, $code) || '';
                substr(
                    $html,
                    $outer_start,
                    $outer_end - $outer_start,
                    $res
                );
            }
        }

        close $fd;
    }

    if ($html =~ /<perl>/){
        $log->fatal("<perl> in '$fname' after processing", {
            html => $html
        });
    }
    if ($html =~ /<\/perl>/){
        $log->fatal("</perl> in '$fname' after processing", {
            html => $html
        });
    }

    $html .= "<!-- $fname } -->\n";

    return $html;
}

## --------------------------------------------------------------------------80

sub processPropsDefaultCb
{
    my $info = shift;
}

sub processProps
{
    my %args = (
        props   => [],
        cb      => \&processPropsDefaultCb,
        @_
    );

    my $row         = 0;
    my $ret         = '';
    my $cur_neuron  = getCurNeuron();
    my $style1      = "background-color: #f0f0f0;";
    my $style2      = "background-color: #e0e0e0;";
    foreach my $prop_id (@{$args{props}}){
        my $prop_val = $cur_neuron->{props}->{$prop_id};
        if (defined $prop_val){
            Encode::_utf8_on($prop_val);
        }

        my $info = {
            style       => (!$row%2) ? $style1 : $style2,
            row         => $row,
            prop_name   => $prop_id,
            prop_id     => $prop_id,
            prop_val    => $prop_val
        };

        ## call cb
        my $cb = $args{cb};
        $cb->($info);

        $ret .= '<tr style="'.$info->{style}.'">';
        $ret .= '<td>'.$info->{prop_name}.'</td>';
        $ret .= '<td>'.$info->{prop_val}.'</td>';
        $ret .= '</tr>';
        $row++;
    }
    return $ret;
}

##
## getCurNeighs - return current neuron neighs
##
## >> param 'type' (string or int)
##
## << array of Neigh objects
##
sub getCurNeighs
{
    my $ret         = [];
    my $cur_neuron  = getCurNeuron();
    if ($cur_neuron){
        $ret = $cur_neuron->getNeighs(@_);
    }
    return $ret;
}

##
## getCurNeighsByType - return neuron neighs by type
##
## >> type (string or int)
##
## << array of Neigh objects
##
sub getCurNeighsByType
{
    my $ret         = [];
    my $cur_neuron  = getCurNeuron();
    if ($cur_neuron){
        $ret = $cur_neuron->getNeighsByType(@_);
    }
    return $ret;
}

sub getHref
{
    my %args = (
        id              => undef,
        object_id       => '',
        view            => undef,
        title           => '',
        href            => undef,
        href_params     => {},
        onclick         => '',
        show_statistic  => 0,
        @_
    );

    my $id              = $args{id};
    my $object_id       = $args{object_id};
    my $href_params     = $args{href_params};
    my $view            = $args{view};
    my $href            = $args{href};
    my $title           = $args{title} || $object_id;
    my $show_statistic  = $args{show_statistic};
    my $onclick         = $args{onclick};
    my $style;

    if (!$href){
        $href = "/api"
    }

    if ($object_id){
        $href_params->{object_id} = $object_id;
    }

    if ($view){
        $href_params->{view} = $view;
    }

    if (!$style){
        my $i = 10 * int rand(10);
        if ($i%5){
            $i = 0;
        }
        $style = "width: ".$i."px;"
            ." height: ".$i."px;"
            ." border-radius: ".$i."px;"
            ." line-height: ".$i."px;";
    }

    my $params  = '';
    my $ret     = '';

    if ($show_statistic){
        $ret .= '<div class="href_container">';
        $ret .=     '<div class="href_stat"';
        $ret .=         (defined $style)    ? " style=\"$style\""   : '';
        $ret .=     '></div>';
        $ret .=     '<div class="href_a">';
    }

    foreach my $href_param (keys %$href_params){
        my $href_param_val = $href_params->{$href_param};
        if (!$params){
            $params .= "?$href_param=$href_param_val";
        } else {
            $params .= "&$href_param=$href_param_val";
        }
    }

    $ret .= "<a href=\"$href$params\"";
    $ret .=     (defined $id) ? " id=\"$id\"" : '';
    if ($onclick){
        $ret .= " onclick=\"$onclick\"";
    }
    $ret .= ">$title</a>";

    if ($show_statistic){
        $ret .=     '</div>';
        $ret .= '</div>';
    }

    return $ret;
}

1;

