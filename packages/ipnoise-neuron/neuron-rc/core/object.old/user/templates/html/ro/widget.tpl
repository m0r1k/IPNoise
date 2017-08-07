<div id="tabs" style="width: 250px">
    <ul>
        <li><a href="#tabs-1">Информация</a></li>
        <li><a href="#tabs-2">Адреса</a></li>
    </ul>
    <div id="tabs-1">
        <form metod="post" id="login">
            <table>
                <perl>
                    my $ret = '';
                    my $params = getCurObjectParams();
                    foreach my $param (@$params){
                        $ret .= $param->renderEmbedded("ro/widget_tr");
                    }
                    return $ret;
                </perl>
            </table>
        </form>
    </div>
    <div id="tabs-2">
        тут будут адреса
    </div>
</div>

