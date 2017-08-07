<div style="width: 100%;"
    class="object">

    <script>
    </script>

    <perl>
        my $ret         = '';
        my $object      = getCurObject();
        my $img_url     = '';

        $img_url = 'static/images/128x128/icon_null_01.png';

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td></td></tr>'
            .'</table>';
        $ret .= getHref(
            title   => $html,
            onclick => 'getCurObject(this).uiAddNew(event);'
        );
        return $ret;
    </perl>
</div>

