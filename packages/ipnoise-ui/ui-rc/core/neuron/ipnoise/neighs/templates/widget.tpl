<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronIPNoiseNeighs">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronIPNoiseNeighsClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <center>
    <h1>IPNoiseNeighs</h1>
    </center>

    <div class="neuron_ipnoise_neighs_add_new">
        <a href="" onclick="getCurNeuron().addNewNeigh();return false;">Добавить нового соседа</a>
    </div>

    <perl>
        my $ret = '';
        $ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

