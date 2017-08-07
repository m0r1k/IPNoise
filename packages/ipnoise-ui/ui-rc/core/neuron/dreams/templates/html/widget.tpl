<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronDreams">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronDreamsClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <center>
    <h1>Dreams</h1>
    </center>

    <div class="neuron_dreams_add_new">
        <a href="" onclick="getCurNeuron().addNewDream();return false;">Добавить новый сон</a>
    </div>

    <perl>
        my $ret = '';
        $ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

