<perl>return processTemplate("core/neuron/neuron/templates/html/admin/body.tpl");</perl>
<h3>Param</h3>
<div>
    <form metod="post" id="updateParam">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $prop_id = $info->{prop_id};
                    if ('name' eq $prop_id){
                        $info->{prop_name} = 'Name';
                    } elsif ('value' eq $prop_id){
                        $info->{prop_name} = 'Value';
                        $info->{prop_val} = '<input type="text" name="value" cols="40"'
                            .' value="'.$info->{prop_val}.'">'
                            .'</input>';
                    };
                };
                my $ret = processProps(
                    props   => ['name', 'value'],
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

