<perl>
    my $ret             = '';
    my $neuron          = getCurNeuron();
    my $neuron_id       = $neuron->getId();
    my $cur_user        = getCurUser();
    my $cur_user_id     = $cur_user->getId();
    my $cur_user_neighs = $cur_user->getNeighs();
    my $found           = 0;

    ## have we such neuron?
    foreach my $cur_neigh (@$cur_user_neighs){
        my $cur_neigh_id = $cur_neigh->getId();
        if ($cur_neigh_id eq $neuron_id){
            $found = 1;
            last;
        }
    }

    ## ignore user's self neuron
    if ($neuron_id eq $cur_user_id){
        $found = 1;
    }

    ## if not - show bookmark icon
    if (!$found){
        my $img_url     = 'static/images/128x128/bookmarks_01.png';
        my $html        = ''
            .'<table class="neuron_icon_bookmark">'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td><nobr>добавить</nobr></td></tr>'
            .'</table>';
        $ret .= getHref(
            title   =>  $html,
            onclick => 'return getCurNeuron().bookmark();'
        );
    }

    return $ret;
</perl>

