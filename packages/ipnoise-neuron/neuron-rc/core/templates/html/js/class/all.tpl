<perl>
    our $modules_info_str;

    if ($modules_info_str){
        return $modules_info_str;
    }

    my $ret             = '';
    my $modules_info    = getModulesInfo();
    my $js_dirs         = [];
    my $types           = {};
    my $types_sorted    = [];

    foreach my $module_type (keys %$modules_info){
        my $module_info = $modules_info->{$module_type};
        my $type        = $module_info->{type};
        my $type_perl   = $module_info->{type_perl};
        if (!$type){
            $log->fatal("type is undefined", {
                'modules_info' => $modules_info
            });
        }
        if (!$type_perl){
            $log->fatal("perl type is undefined", {
                'modules_info' => $modules_info
            });
        }
        $types->{$type} = {
            'type'      => $type,
            'type_perl' => $type_perl
        };
        push (@$types_sorted, $type);
    }

    ## sort by name length, short names will process first
    @$types_sorted = sort {
        split("\\.", $a) <=> split("\\.", $b)
    } @$types_sorted;

    foreach my $type (@$types_sorted){
        foreach my $layer ('', '/html', '/webgl'){
            my $info        = $types->{$type};
            my $type_perl   = $info->{type_perl};

            my @tmp = split("::", $type_perl);
            pop @tmp; ## remove "main"

            my $module_dir  = join("/", @tmp);
            my $js_dir      = "$module_dir/templates"
                .$layer
                ."/js/class";
            push (@$js_dirs, $js_dir);
        }
    }

    foreach my $js_dir (@$js_dirs){
        my $dh;

        ## read classes
        opendir $dh, $js_dir
            or  $log->fatal("could not open dir '$js_dir'"
                    ." for read ($!)"
                );

        my @inodes = readdir $dh;
        foreach my $inode (@inodes){
            if (    !$inode
                ||  $inode !~ /\.tpl$/
                ||  $inode eq 'all.tpl')
            {
                next;
            }
            my $fname = "$js_dir/$inode";
            $ret .= processTemplate($fname);
        }
        closedir $dh;
    }

    $modules_info_str = $ret;

    return $ret;
</perl>

