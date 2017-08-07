<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronOrdersClass();
            neuron.setId(
                "<perl>return getCurNeuronId()</perl>"
            );
            neuron.load();
            neuron.render();
        });
    </script>

    <perl>
        my $ret     = '';
        my $orders  = getCurNeighsByType("order");
        foreach my $order (@$orders){
            $ret .= '<hr/>';
            $ret .= '<table>';
            $ret .= $order->renderEmbedded("widget_tr");
            $ret .= '</table>';
        }
        return $ret;
    </perl>
</div>

