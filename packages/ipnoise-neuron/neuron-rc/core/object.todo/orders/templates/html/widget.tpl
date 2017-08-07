<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object">

    <script>
    </script>

    <perl>
        my $ret     = '';
        my $orders  = getCurNeighsByType("order");
        foreach my $order (@$orders){
            $ret .= '<hr/>';
            $ret .= '<table>';
            $ret .= $order->renderEmbedded("widget_tr");
            $ret .= '</table>';
        }
        return $ret;
    </perl>
</div>

