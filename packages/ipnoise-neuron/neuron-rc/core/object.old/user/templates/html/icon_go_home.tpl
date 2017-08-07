<perl>
    my $ret     = '';
    my $object  = getCurUser();
    my $html    = ''
        .'<table>'
            .'<tr><td>'
                .'<img width="32"'
                .' src="static/images/128x128/go_home_01.png"'
                .'/>'
            .'</td></tr>'
            .'<tr><td>Домой</td></tr>'
        .'</table>';
    $ret .= getHref(
        object_id   =>  $object->getId(),
        title       =>  $html
    );
    return $ret;
</perl>

