<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object">

<perl>
    my $ret     = '';
    my $orders  = getCurNeighsByType("order");
    $ret .= '<table>';
    $ret .= '<tr>';
    $ret .=     '<th>Дата</th>';
    $ret .=     '<th>Количество товаров</th>';
    $ret .=     '<th>Сумма</th>';
    $ret .=     '<th>Статус</th>';
    $ret .=     '<th>Подробнее</th>';
    $ret .= '</tr>';
    foreach my $order (@$orders){
        $ret .= $order->renderEmbedded("short_tr");
    }
    $ret .= '</table>';
    return $ret;
</perl>

</div>

