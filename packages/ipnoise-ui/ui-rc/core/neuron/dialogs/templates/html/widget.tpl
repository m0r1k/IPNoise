<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronDialogs">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronDialogsClass();
            neuron.setId(
                "<perl>return getCurNeuronId()</perl>"
            );
            neuron.load();
            neuron.render();
        });
    </script>

    <div class="neuron_dialogs_add_new">
        <a href="" onclick="getCurNeuron().openNewDialog();return false;">Создать новый диалог</a>
    </div>

    <perl>
        my $ret = '';
        $ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

