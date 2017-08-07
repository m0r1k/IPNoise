<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>IPNoiseSkb</h3>
<div>
    <form metod="post" id="updateSkb">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $object_id = $info->{object_id};
                    if ('src_dev_index' eq $object_id){
                        $info->{object_name} = 'src_dev_index';
                        $info->{object_val} = '<input type="text"'
                            .' name="src_dev_index"'
                            .' size="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    objects   => [ 'src_dev_index' ],
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

