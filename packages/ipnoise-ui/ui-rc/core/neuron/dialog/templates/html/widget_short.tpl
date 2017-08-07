<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronDialog">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronDialogClass();
            neuron.setId(
                "<perl>return getCurNeuronId()</perl>"
            );
            neuron.load();
            neuron.render();
        });
    </script>

    <table>
        <tr>
            <td>Dialog short info</td>
        </tr>
    </table>

</div>

