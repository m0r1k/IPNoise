<perl>return processTemplate("core/neuron/neuron/templates/html/admin/body.tpl");</perl>
<h3>IPNoiseSkb</h3>
<div>
    <form metod="post" id="updateSkb">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $prop_id = $info->{prop_id};
                    if ('src_dev_index' eq $prop_id){
                        $info->{prop_name} = 'src_dev_index';
                        $info->{prop_val} = '<input type="text"'
                            .' name="src_dev_index"'
                            .' size="40"'
                            .' value="'.$info->{prop_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    props   => [ 'src_dev_index' ],
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

