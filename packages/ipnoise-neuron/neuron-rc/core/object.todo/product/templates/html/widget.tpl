<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object objectProduct">

    <script>
    </script>

    <table class="object_product_widget">
        <tr>
            <td></td>
            <td class="object_product_widget_title">
                <h1><perl>return getCurObject()->getTitle()</perl></h1>
            </td>
            <td class="object_product_widget_basket">
                <perl>
                    my $ret     = '';
                    my $html    = ''
                        .'<table>'
                            .'<tr><td>'
                                .'<img width="32"'
                                .' src="static/images/128x128/basket_add_01.png"'
                                .'/>'
                            .'</td></tr>'
                            .'<tr><td>Добавить в корзину</td></tr>'
                        .'</table>';
                    $ret .= getHref(
                        id      =>  "basket_add",
                        href    =>  "#",
                        onclick =>  'uiGetObject($(this)).addToBasket()',
                        title   =>  $html
                    );
                </perl>
            </td>
        </tr>
        <tr>
            <td></td>
            <td class="object_product_widget_photos">
                <perl>
                    my $ret         = '';
                    my $neighs      = getCurNeighsByType("photo");
                    my $user        = getCurUser();
                    foreach my $neigh (@$neighs){
                        my %args = (
                            object_id   => $neigh->getId(),
                            title       => $neigh->renderEmbedded("small")
                        );
                        $ret .= getHref(%args);
                    }
                    return $ret;
                </perl>
            </td>
            <td></td>
        </tr>
    </table>
</div>
