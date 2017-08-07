<perl>
    my $ret     = '';
    my $session = getSession();

    if ($session->isRegistered()){
        my $html    = ''
            .'<table>'
                .'<tr><td>'
                    .'<img width="32"'
                    .' src="static/images/128x128/power_red_01.png"'
                    .'/>'
                .'</td></tr>'
                .'<tr><td>Выйти</td></tr>'
            .'</table>';
        $ret .= getHref(
            id      =>  "logout_dialog",
            href    =>  "#",
            title   =>  $html
        );
    } else {
        my $html    = ''
            .'<table>'
                .'<tr><td>'
                    .'<img width="32"'
                    .' src="static/images/128x128/login_01.png"'
                    .'/>'
                .'</td></tr>'
                .'<tr><td>Войти</td></tr>'
            .'</table>';
        $ret .= getHref(
            id      =>  "login_dialog",
            href    =>  "#",
            title   =>  $html
        );
    }
    return $ret;
</perl>

