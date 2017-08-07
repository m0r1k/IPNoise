<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>IPNoiseLink</h3>
<div>
    <form metod="post" id="updateLink">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $object_id = $info->{object_id};
                    if ('if_index' eq $object_id){
                        $info->{object_name} = 'if_index';
                        $info->{object_val}  = '<input type="text"'
                            .' name="if_index"'
                            .' size="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    } elsif ('check_time' eq $object_id){
                        $info->{object_name} = 'check_time';
                        $info->{object_val}  = '<input type="text"'
                            .' name="check_time"'
                            .' size="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    objects   => [ 'if_index', 'check_time' ],
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

