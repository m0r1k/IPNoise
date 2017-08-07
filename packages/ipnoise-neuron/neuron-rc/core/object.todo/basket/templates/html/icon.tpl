<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object object_icon">

    <script>
    </script>

    <perl>
        my $ret         = '';
        my $object      = getCurObject();
        my $products    = $object->getNeighsByType('product');
        my $img_url     = '';

        if (scalar(@$products)){
            $img_url = 'static/images/128x128/basket_not_empty_01.png';
        } else {
            $img_url = 'static/images/128x128/basket_empty_01.png';
        }

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td><nobr>Корзина ('.scalar(@$products).')</nobr></td></tr>'
            .'</table>';
        $ret .= getHref(
            object_id   =>  $object->getId(),
            title       =>  $html
        );
        return $ret;
    </perl>
</div>

