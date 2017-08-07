<perl>return processTemplate("core/neuron/neuron/templates/html/admin/body.tpl");</perl>
<h3>IPNoiseLink</h3>
<div>
    <form metod="post" id="updateLink">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $prop_id = $info->{prop_id};
                    if ('if_index' eq $prop_id){
                        $info->{prop_name} = 'if_index';
                        $info->{prop_val}  = '<input type="text"'
                            .' name="if_index"'
                            .' size="40"'
                            .' value="'.$info->{prop_val}.'">'
                            .'</input>';
                    } elsif ('check_time' eq $prop_id){
                        $info->{prop_name} = 'check_time';
                        $info->{prop_val}  = '<input type="text"'
                            .' name="check_time"'
                            .' size="40"'
                            .' value="'.$info->{prop_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    props   => [ 'if_index', 'check_time' ],
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

