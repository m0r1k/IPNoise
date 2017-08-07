<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

<script>
    $(document).ready(function(){
        // setup neuron
        var neuron = NeuronDialogMsgClass();
        neuron.setId(
            "<perl>return getCurNeuronId()</perl>"
        );
        neuron.load();
        neuron.render();
    });
</script>

<tr>
    <td>2013.09.10</td>
    <td>tigra</td>
    <td>
        <perl>
            my $ret     = '';
            my $neuron  = getCurNeuron();
            $ret .= getHref(
                object_id   =>  $neuron->getId(),
                title       =>  'подробнее'
            );
        </perl>
    </td>
</tr>

</div>

