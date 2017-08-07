<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron">

    <script>
        $(document).ready(function(){
            // setup
            var user = NeuronUserClass();
            user.setId("<perl>return getCurNeuronId()</perl>");
            user.load();
            user.render();
        });
    </script>

    <h1>Хомяк</h1>

    <img class="load" width="100" load_url="/static/images/128x128/basket_add_01.png"/>

    <ul>
        <li>
        <perl>
            my $ret     = '';
            my $user    = getCurUser();
            my $dialogs = $user->getNeighByType(
                "core.neuron.dialogs"
            );
            if ($dialogs){
                $ret .= getHref(
                    object_id   =>  $dialogs->getId(),
                    title       =>  'Мои диалоги'
                );
            }
            return $ret;
        </perl>
        </li>
    </ul>

</div>

