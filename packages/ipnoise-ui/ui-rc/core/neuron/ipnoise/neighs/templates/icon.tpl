<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuron_icon">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronIPNoiseNeighsClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            var script_el = document.scripts[
                document.scripts.length - 1
            ];
            var parent_el = script_el.parentNode;
            neuron.setRenderTo($(parent_el));
            neuron.setView('icon');
            neuron.load();
            neuron.render();
        });
    </script>

    <perl>
        my $ret         = '';
        my $neuron      = getCurNeuron();
        my $img_url     = '';

        $img_url = 'static/images/128x128/ipnoise_neighs_01.png';

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td>IPNoiseNeighs</td></tr>'
            .'</table>';
        $ret .= getHref(
            object_id   =>  $neuron->getId(),
            title       =>  $html
        );
        return $ret;
    </perl>
</div>

