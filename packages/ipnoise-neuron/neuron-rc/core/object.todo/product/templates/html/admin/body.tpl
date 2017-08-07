<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>Product</h3>
<div>
    <form metod="post" id="updateProduct">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $object_id = $info->{object_id};
                    if ('title' eq $object_id){
                        $info->{object_name} = 'Title';
                        $info->{object_val} = '<textarea name="title" cols="40">'
                            .$info->{object_val}.'</textarea>';
                    }
                };
                my $ret = processProps(
                    objects   => [ 'title' ],
                    cb      => $cb
                );
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

