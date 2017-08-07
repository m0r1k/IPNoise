<div style="width: 100%;"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });

    </script>

    <perl>
        my $ret         = '';
        my $neuron      = getCurNeuron();
        my $img_url     = '';

        $img_url = 'static/images/128x128/icon_null_01.png';

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td></td></tr>'
            .'</table>';
        $ret .= getHref(
            title   => $html,
            onclick => 'getCurNeuron(this).uiAddNew(event);'
        );
        return $ret;
    </perl>
</div>

