<table width="100%" class="header">

<tr>
<td>
<perl>
    my $ret     = '';
    my $object  = getCurObject();
    $ret        = $object->renderEmbedded('icon_map');
    return $ret;
</perl>
</td>
<perl>
    my $ret     = '';
    my $object  = getCurObject();
    $ret        = $object->renderEmbedded('icon_bookmark_td');
    return $ret;
</perl>
<td width="100%"></td>
<td>
<perl>
    my $object = getCurObject();
    my $ret    = $object->renderEmbedded('icons');
    return $ret;
</perl>
</td>
<td>
<perl>
    my $ret = '';
    {
        my $session = getSession();
        if (!$session){
            next;
        }
        my $user = $session->getUser();
        if (!$user){
            next;
        }
        $ret .= $user->renderEmbedded('icon_session');
    }
    return $ret;
</perl>
</td>
<td>
<div id="login_dialog" style="display: none;">
<div id="login_tabs">
    <ul>
        <li><a href="#tabs-1">Login</a></li>
        <li><a href="#tabs-2">Registration</a></li>
    </ul>
    <div id="tabs-1">
        <form metod="post" id="login">
            <table>
                <tr>
                    <td>Login:</td>
                    <td><input type="login" name="login"></input></td>
                </tr>
                <tr>
                    <td>Password:</td>
                    <td><input type="password" name="password"></input></td>
                </tr>
                <tr>
                    <td>
                        <input type="submit" value="enter"></input>
                    </td>
                    <td></td>
                </tr>
            </table>
        </form>
    </div>
    <div id="tabs-2">
        <form method="post" id="register">
            <table>
                <tr>
                    <td>Login:</td>
                    <td><input type="login" name="login"></input></td>
                </tr>
                <tr>
                    <td>Password:</td>
                    <td><input type="password" name="password1"></input></td>
                </tr>
                <tr>
                    <td>Password (again):</td>
                    <td><input type="password" name="password2"></input></td>
                </tr>
                <tr>
                    <td>
                        <input type="submit" value="enter"></input>
                    </td>
                    <td></td>
                </tr>
            </table>
        </form>
    </div>
</div>
</div>

</td>
</tr>

</table>

