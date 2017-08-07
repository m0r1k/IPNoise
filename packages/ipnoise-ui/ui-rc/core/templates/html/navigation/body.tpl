<table width="100%" class="navigation">
<tr>
<td>
<perl>
    my $ret         = '';
    my $cur_neuron  = getCurNeuron();
    my $neighs      = $cur_neuron->getNeighs(
        type => [ 'shop', 'photo', 'products', 'product' ]
    );
    foreach my $neigh (@$neighs){
        if ($ret){
            $ret .= ", ";
        }
        my %args = (
            object_id   => $neigh->getId(),
            title       => $neigh->getNavName()
        );
        $ret .= getHref(%args);
    }
    return $ret;
</perl>
</td>
</tr>
</table>

