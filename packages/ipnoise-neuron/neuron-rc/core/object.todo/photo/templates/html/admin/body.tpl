<perl>return processTemplate("core/object/object/templates/html/admin/body.tpl");</perl>
<h3>Photo</h3>
<div>
    <form metod="post" id="updatePhoto">
        <table>
            <perl>
                my $cb = sub {
                    my $info    = shift;
                    my $object_id = $info->{object_id};
                    if ('title' eq $object_id){
                        $info->{object_name} = 'Title';
                        $info->{object_val} = '<textarea name="title" cols="40">'
                            .$info->{object_val}.'</textarea>';
                    } elsif ('url' eq $object_id){
                        $info->{object_name} = 'URL';
                        $info->{object_val} = '<input type="text" name="url" size="40"'
                            .' value="'.$info->{object_val}.'">'
                            .'</input>';
                    }
                };
                my $ret = processProps(
                    objects   => [ 'title', 'url' ],
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

