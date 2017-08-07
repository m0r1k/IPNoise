<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronDialogs">

<perl>
    my $ret     = '';
    my $dialogs = getCurNeighsByType("dialog");
    $ret .= '<table>';
    $ret .= '<tr>';
    $ret .=     '<th>Дата</th>';
    $ret .=     '<th>Количество сообщений</th>';
    $ret .=     '<th>Подробнее</th>';
    $ret .= '</tr>';
    foreach my $dialog (@$dialogs){
        $ret .= $dialog->renderEmbedded("short_tr");
    }
    $ret .= '</table>';
    return $ret;
</perl>

</div>

