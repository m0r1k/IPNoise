<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuron_icon">

    <perl>
        my $ret         = '';
        my $neuron      = getCurNeuron();
        my $img_url     = '';

        $img_url = $neuron->getUrl();

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td>Photo</td></tr>'
            .'</table>';
        $ret .= getHref(
            object_id   =>  $neuron->getId(),
            title       =>  $html
        );
        return $ret;
    </perl>
</div>

