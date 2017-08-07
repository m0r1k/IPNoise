<perl>return processTemplate("core/neuron/neuron/templates/html/admin/body.tpl");</perl>
<h3>Products</h3>
<div>
    <form metod="post" id="updateProducts">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $prop_id = $info->{prop_id};
                    if ('title' eq $prop_id){
                        $info->{prop_name} = 'Title';
                        $info->{prop_val} = '<textarea name="title" cols="40">'
                            .$info->{prop_val}.'</textarea>';
                    }
                };
                my $ret = processProps(
                    props   => [ 'title' ],
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

