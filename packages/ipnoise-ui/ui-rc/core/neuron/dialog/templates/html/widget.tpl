<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronDialogClass();
            neuron.setId(
                "<perl>return getCurNeuronId()</perl>"
            );
            neuron.load();
            neuron.render();

            // setup handlers
            $('.neuron_dialog_controls_submit').click(function(ev){
                getCurNeuron(this).send(this, ev);
            });
        });
    </script>

    <table class="neuron_dialog_widget">
        <tr>
             <td>
                  <div class="neuron_dialog_messages">
                  <perl>
                      my $ret       = '';
                      my $neuron    = getCurNeuron();
                      my $msgs      = $neuron->getNeighsByType('dialog_msg');

                      foreach my $msg (@$msgs){
                          $ret .= $msg->renderEmbedded('widget');
                      }

                      return $ret;
                  </perl>
                  </div>
            </td>
        </tr>
        <tr>
            <td>
                <table class="neuron_dialog_controls">
                    <tr>
                        <td>
                            <textarea class="neuron_dialog_controls_input"></textarea>
                        </td>
                    </tr>
                    <tr>
                        <td>
                            <input type="button" value="Отправить" class="neuron_dialog_controls_submit"/>
                        </td>
                    </tr>
                </table>
            </td>
        </tr>
    </table>
</div>

