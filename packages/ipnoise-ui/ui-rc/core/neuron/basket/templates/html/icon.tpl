<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuron_icon">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronBasketClass();
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
        my $products    = $neuron->getNeighsByType('product');
        my $img_url     = '';

        if (scalar(@$products)){
            $img_url = 'static/images/128x128/basket_not_empty_01.png';
        } else {
            $img_url = 'static/images/128x128/basket_empty_01.png';
        }

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td><nobr>Корзина ('.scalar(@$products).')</nobr></td></tr>'
            .'</table>';
        $ret .= getHref(
            object_id   =>  $neuron->getId(),
            title       =>  $html
        );
        return $ret;
    </perl>
</div>

