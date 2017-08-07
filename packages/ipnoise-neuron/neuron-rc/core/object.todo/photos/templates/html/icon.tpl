<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object object_icon">

    <script>
    </script>

    <perl>
        my $ret         = '';
        my $object      = getCurObject();
        my $img_url     = '';

        $img_url = $object->getUrl();

        my $html = ''
            .'<table>'
                .'<tr><td>'
                    .'<img src="'.$img_url.'"/>'
                .'</td></tr>'
                .'<tr><td>Photos</td></tr>'
            .'</table>';
        $ret .= getHref(
            object_id   =>  $object->getId(),
            title       =>  $html
        );
        return $ret;
    </perl>
</div>

