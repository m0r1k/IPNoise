<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron icon_map header_icon">

    <script>
        $(document).ready(function(){
            function select(a_event){
                var id = "";

                // get target ui elemet
                var target = $(a_event.target
                    || a_event.srcElement
                );

                // get id
                id = target.parents('.neuron').attr("id");

                // prevent default browser handler
                a_event.preventDefault();

                // stop event transmittion
                // to upper elements
                a_event.stopPropagation();

                // remove old map
                var panel        = $(".neuron_user_map");
                var panel_parent = panel.parent();
                panel.remove();

                // create fake neuron
                var neuron = NeuronClass();
                neuron.setId(id);
                neuron.renderEmbeddedTo(
                    panel_parent,
                    "map"
                );
            }
            $(".map_flip").click(function(a_event){
                var panel = $(".map_panel");
                if (!panel.is(":visible")){
                    select(a_event);
                }
                panel.slideToggle("slow", function(a_event){});
                //$(".center").fadeTo(
                //    "slow",
                //    0.9,
                //    function(){}
                //);
            });
            $(".map_panel_controls").find('a').click(
                function(a_event){
                    select(a_event);
                }
            );
        });
    </script>

    <div class="map_flip">
    <perl>
        my $ret         = '';
        my $neuron      = getCurNeuron();
        my $img_url     = '';

        $img_url = 'static/images/128x128/map_01.png';

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td>Map</td></tr>'
            .'</table>';
        $ret .= "$html";

#        $ret .= getHref(
#            title   => $html,
#            onclick => "alert(11);"
#        );
        return $ret;
    </perl>
    </div>

    <div class="map_panel">
    <table class="map_panel_table">
        <tr>
            <td class="map_panel_controls">
                <table>
                    <tr><td>
                    <perl>
                        my $ret  = '';
                        my $user = getCurUser();
                        $ret .= $user->renderEmbedded('icon');
                        return $ret;
                    </perl>
                    </td></tr>
                    <tr><td>
                    <perl>
                        my $ret    = '';
                        my $user   = getCurUser();
                        my $neuron = getCurNeuron();
                        if ($user->getId() ne $neuron->getId()){
                            $ret .= $neuron->renderEmbedded('icon');
                        }
                        return $ret;
                    </perl>
                    </td></tr>
                </table>
            </td>
            <td>
                <perl>
                    my $ret = processCurTemplate("map.tpl");
                    return $ret;
                </perl>
            </td>
            <td></td>
        </tr>
        <tr>
            <td></td>
            <td></td>
            <td></td>
        </tr>
    </table>
    </div>

</div>

