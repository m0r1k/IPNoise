<perl>
    my $ret          = '';
    my $neurons_info = getNeuronsInfo();

    foreach my $neuron_type (keys %$neurons_info){
        my $dh;
        my $neuron_dir  = $neuron_type;
        $neuron_dir     =~ s/\./\//g;
        my $js_dir      = "$neuron_dir/js/class/";

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

    return $ret;
</perl>

