<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>User</h3>
<div>
    <form metod="post" id="updateUser">
        <table>
            <perl>
                my $ret = '';
                my $params = getCurObjectParams();
                foreach my $param (@$params){
                    my $view = 'widget_tr';
                    if ('login' eq $param->getName()){
                        $view = 'ro/widget_tr';
                    }
                    $ret .= $param->renderEmbedded($view);
                }
                return $ret;
            </perl>
            <tr>
                <td>
                    <input type="submit" value="Сохранить"></input>
                </td>
                <td></td>
            </tr>
        </table>
    </form>
</div>

