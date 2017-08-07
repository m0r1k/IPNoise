<perl>
    my $ret    = '';
    my $dreams = getCurNeighsByType("dream");

    $ret .= '<div class="object_dreams_list">';
    $ret .=     '<div class="object_dreams_list_title">';
    $ret .=         '<h2>Сны:</h2>';
    $ret .=     '</div>';

    if (scalar(@$dreams)){
        $ret .=     '<div>';
        $ret .=         '<ul class="object_dreams_list_content">';
        $ret .=             '<table>';
        foreach my $cur_dream (@$dreams){
            $ret .= $cur_dream->renderEmbedded("widget_tr");
        }
        $ret .=             '</table>';
        $ret .=         '</ul>';
    } else {
        $ret .= 'У Вас нет пока снов';
    }

    $ret .=     '</div>';
    $ret .= '</div>';

    return $ret;
</perl>

