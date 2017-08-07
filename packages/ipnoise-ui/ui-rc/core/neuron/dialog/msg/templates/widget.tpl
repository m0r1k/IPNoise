<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronDialogMsg">

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

    <table class="neuron_dialog_msg">
        <tr>
            <perl>
                my $ret     = '';
                my $neuron  = getCurNeuron();
                my $date    = $neuron->getDate();
                my $login   = $neuron->getUserLogin();
                my $text    = $neuron->getText();

                $ret .= '<td>'.$date.'</td>';
                $ret .= '<td>'.$login.'</td>';
                $ret .= '<td class="neuron_dialog_msg_text">'.$text.'</td>';

                return $ret;
            </perl>
        </tr>
    </table>
</div>

