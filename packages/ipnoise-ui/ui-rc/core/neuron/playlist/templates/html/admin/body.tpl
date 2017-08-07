<perl>return processTemplate("core/neuron/neuron/templates/html/admin/body.tpl");</perl>
<h3>Playlist</h3>
<div>
    <form metod="post" id="updatePlaylist">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $prop_id = $info->{prop_id};
                    if ('status' eq $prop_id){
                        $info->{prop_name} = 'Status';
                        $info->{prop_val} = '<input type="text" name="status" size="40"'
                            .' value="'.$info->{prop_val}.'">'
                            .'</input>';
                    } elsif ('play_order' eq $prop_id){
                        $info->{prop_name} = 'Play order';
                        $info->{prop_val} = '<input type="text" name="play_order" size="40"'
                            .' value="'.$info->{prop_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    props   => [ 'status', 'play_order' ],
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

