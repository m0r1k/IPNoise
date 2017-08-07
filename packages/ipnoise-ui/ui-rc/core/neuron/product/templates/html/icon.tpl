<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuron_icon">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronProductClass();
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
        my $neighs      = getCurNeighsByType("photo");
        my $title       = $neuron->getTitle();

        if (@$neighs){
            $img_url = $neighs->[0]->getUrl();
        }

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td>'.$title.'</td></tr>'
            .'</table>';
        $ret .= getHref(
            object_id   =>  $neuron->getId(),
            title       =>  $html
        );
        return $ret;
    </perl>
</div>

