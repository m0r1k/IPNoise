<div    class="NeuronPhotos"
        neuron_id="<perl>return getCurNeuronId()</perl>">

    <perl>
        my $ret    = '';
        my $neuron = getCurNeuron();
        my $photos = $neuron->getNeighsByType("core.neuron.photo");
        my $photo;

        if (scalar(@$photos)){
            $photo = $photos->[0];
            $ret .= $photo->getId().'<br/>';
            $ret .= $photo->renderEmbedded('widget');

            $ret .= '<br/>';

            $photo = $photos->[0];
            $ret .= $photo->getId().'<br/>';
            $ret .= $photo->renderEmbedded('widget');

            $ret .= '<br/>';

            $photo = $photos->[0];
            $ret .= $photo->getId().'<br/>';
            $ret .= $photo->renderEmbedded('widget');
        }

        return $ret;
    </perl>
</div>

