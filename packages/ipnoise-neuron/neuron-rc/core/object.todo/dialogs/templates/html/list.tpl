<perl>
    my $ret    = '';
    my $dialogs = getCurNeighsByType("dialog");

    $ret .= '<div class="object_dialogs_list">';
    $ret .=     '<div class="object_dialogs_list_title">';
    $ret .=         '<h2>Диалоги:</h2>';
    $ret .=     '</div>';

    if (scalar(@$dialogs)){
        $ret .=     '<div>';
        $ret .=         '<ul class="object_dialogs_list_content">';
        $ret .=             '<table>';
        foreach my $cur_dialog (@$dialogs){
            $ret .= $cur_dialog->renderEmbedded("widget_short_tr");
        }
        $ret .=             '</table>';
        $ret .=         '</ul>';
    } else {
        $ret .= 'У Вас нет пока диалогов';
    }

    $ret .=     '</div>';
    $ret .= '</div>';

    return $ret;
</perl>

