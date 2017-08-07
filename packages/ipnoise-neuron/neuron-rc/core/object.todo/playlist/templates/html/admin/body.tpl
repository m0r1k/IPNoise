<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>Playlist</h3>
<div>
    <form metod="post" id="updatePlaylist">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $object_id = $info->{object_id};
                    if ('status' eq $object_id){
                        $info->{object_name} = 'Status';
                        $info->{object_val} = '<input type="text" name="status" size="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    } elsif ('play_order' eq $object_id){
                        $info->{object_name} = 'Play order';
                        $info->{object_val} = '<input type="text" name="play_order" size="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    objects   => [ 'status', 'play_order' ],
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

