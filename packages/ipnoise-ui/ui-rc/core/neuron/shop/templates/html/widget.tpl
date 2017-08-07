<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronShopClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <center>
        <h1>Toscana</h1>
        <perl>
            my $ret         = '';
            my $img         = '<img'
                .' width="900px"'
                .' src="/static/images/Amalfi_Italy_4.jpg"'
                .'/>';
            my %args = (
                object_id   => 'f1b6a33043bf404e0c6c0637ea86c91eecb289ef',
                title       => $img
            );
            $ret .= getHref(%args);
            return $ret;
        </perl>
    </center>
</div>

