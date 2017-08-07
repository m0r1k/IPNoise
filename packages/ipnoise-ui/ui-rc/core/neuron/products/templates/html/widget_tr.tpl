<tr>
    <td>
    <perl>
        my $cur_neuron  = getCurNeuron();
        my $ret         = '';
        my $neighs      = getCurNeighsByType("photo");
        foreach my $neigh (@$neighs){
            $ret .= getHref(
                object_id   =>  $cur_neuron->getId(),
                title       =>  $neigh->renderEmbedded("small")
            );
            last;
        }
        return $ret;
    </perl>
    </td>
    <td>
    <perl>
        my $cur_neuron  = getCurNeuron();
        my $ret         = '';
        $ret .= getHref(
            object_id   => $cur_neuron->getId(),
            title       => $cur_neuron->getTitle()
        );
        return $ret;
    </perl>
    </td>
</tr>

