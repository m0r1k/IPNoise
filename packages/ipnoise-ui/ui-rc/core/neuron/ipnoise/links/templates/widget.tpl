<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronIPNoiseLinks">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronIPNoiseLinksClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <center>
    <h1>IPNoiseLinks</h1>
    </center>

    <div class="neuron_ipnoise_neighs_add_new">
        <a href="" onclick="getCurNeuron().addNewLink();return false;">Добавить нового соседа</a>
    </div>

    <perl>
        my $ret = '';
        #$ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

