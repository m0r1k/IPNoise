<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>Param</h3>
<div>
    <form metod="post" id="updateParam">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $object_id = $info->{object_id};
                    if ('name' eq $object_id){
                        $info->{object_name} = 'Name';
                    } elsif ('value' eq $object_id){
                        $info->{object_name} = 'Value';
                        $info->{object_val} = '<input type="text" name="value" cols="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    };
                };
                my $ret = processProps(
                    objects   => ['name', 'value'],
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

