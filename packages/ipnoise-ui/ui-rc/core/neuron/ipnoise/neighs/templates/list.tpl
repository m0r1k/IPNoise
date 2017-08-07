<perl>
    my $ret            = '';
    my $ipnoise_neighs = getCurNeighsByType("ipnoise_neigh");

    $ret .= '<div class="neuron_ipnoise_neighs_list">';
    $ret .=     '<div class="neuron_ipnoise_neighs_list_title">';
    $ret .=         '<h2>Соседи:</h2>';
    $ret .=     '</div>';

    if (scalar(@$ipnoise_neighs)){
        $ret .=     '<div>';
        $ret .=         '<ul class="neuron_ipnoise_neighs_list_content">';
        $ret .=             '<table>';
        foreach my $cur_ipnoise_neigh (@$ipnoise_neighs){
            $ret .= $cur_ipnoise_neigh->renderEmbedded("widget_tr");
        }
        $ret .=             '</table>';
        $ret .=         '</ul>';
    } else {
        $ret .= 'У Вас нет пока соседей';
    }

    $ret .=     '</div>';
    $ret .= '</div>';

    return $ret;
</perl>

