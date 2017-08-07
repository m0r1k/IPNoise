<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronDreamlandClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <table width="100%" class="neuron_dreamland">
        <tr>
            <td width="100%"><center><h1>Dreamland</h1></center></td>
            <td></td>
        </tr>
        <tr>
            <td class="neuron_dreamland_descr">
                <ul><li>
                    <perl>
                        my $ret     = '';
                        my $user    = getCurUser();
                        my $dreams  = $user->getNeighByType(
                            "core.neuron.dreams"
                        );
                        $ret .= getHref(
                            object_id   =>  $dreams->getId(),
                            title       =>  'открыть дневник'
                        );
                        return $ret;
                    </perl>
                </li></ul>
            </td>
        </tr>
    </table>
</div>

