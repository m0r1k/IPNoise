<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

<script>
    $(document).ready(function(){
        // setup dialog
        var dialog = NeuronDialogClass();
        dialog.setId(
            "<perl>return getCurNeuronId()</perl>"
        );
        dialog.load();
        dialog.render();
    });
</script>

<tr>
    <td>2013.09.10</td>
    <td>0</td>
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

