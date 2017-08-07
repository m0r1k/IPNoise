<perl>
    my $ret           = '';
    my $ipnoise_links = getCurNeighsByType("ipnoise_link");

    $ret .= '<div class="neuron_ipnoise_links_list">';
    $ret .=     '<div class="neuron_ipnoise_links_list_title">';
    $ret .=         '<h2>Соседи:</h2>';
    $ret .=     '</div>';

    if (scalar(@$ipnoise_links)){
        $ret .=     '<div>';
        $ret .=         '<ul class="neuron_ipnoise_links_list_content">';
        $ret .=             '<table>';
        foreach my $cur_ipnoise_link (@$ipnoise_links){
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

