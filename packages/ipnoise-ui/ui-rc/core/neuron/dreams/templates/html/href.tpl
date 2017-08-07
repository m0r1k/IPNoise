<perl>
    my $ret         = '';
    my $cur_neuron  = getCurNeuron();
    my $title       = $cur_neuron->getTitle();
    my %args = (
        object_id   => $cur_neuron->getId(),
        title       => $title
    );
    $ret .= getHref(%args);

    return $ret;
</perl>

