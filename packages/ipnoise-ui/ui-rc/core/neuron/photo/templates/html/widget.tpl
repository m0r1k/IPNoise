<div    class="NeuronPhoto"
        neuron_id="<perl>return getCurNeuronId()</perl>">
</div>

<!--
    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronPhotoClass();
            neuron.setId(
                "<perl>return getCurNeuronId()</perl>"
            );
            neuron.load();
            neuron.render();
        });
    </script>

    <h1>Фото:
        <perl>
            my $ret = "\"".getCurNeuron()->getTitle()."\"";
            return $ret;
        </perl>
    </h1>

    <a href="/api?object_id=<perl>
            return getCurNeuron()->getId();
        </perl>&view=full"
        target="_blank">

        <img style="width: 600px" src="<perl>
            return getCurNeuron()->getUrl();
        </perl>"/>
    </a>

    <perl1>
        my $ret      = '';
        my $cur_user = getCurUser();
        if ($cur_user->isAdmin()){
            $ret .= processCurTemplate("upload.tpl");
        }
        return $ret;
    </perl1>
-->

