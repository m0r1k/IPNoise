<h3>Elementary</h3>
<div>
    <form metod="post" id="updateElementary">
        <table>
            <perl>
                my $cb = sub {
                    my $info = shift;

                    my $cur_object  = getCurObject();
                    my $type        = $cur_object->getType();
                    my $object_id     = $info->{object_id};

                    if ('_id' eq $object_id){
                        $info->{object_name} = "ID";
                        $info->{object_val}  = '<input'
                            .' type="text" size="40"'
                            .' name="id"'
                            .' style="background-color: #e0e0e0;" readonly'
                            .' value="'.$cur_object->getId().'">'
                            .'</input>';
                    } elsif ('type' eq $object_id){
                        $info->{object_id}   = 'Type';
                    } elsif ('name' eq $object_id){
                        $info->{object_name} = "Name";
                        $info->{object_val}  = '<input'
                            .' type="text" size="40"'
                            .' name="name"'
                            .' style="background-color: #e0e0e0;"'
                            .' value="'.$cur_object->getName().'">'
                            .'</input>';
                    } elsif ('neighs' eq $object_id){
                        $info->{object_name} = 'Neighs';
                        $info->{object_val}  = '<table'
                            .' style="background-color: #c0c0c0"'
                            .' width="100%"'
                            .'>';
                        $info->{object_val} .= '<tr>';
                        $info->{object_val} .=    '<th style="width: 100%">ID</th>';
                        $info->{object_val} .=    '<th>Type</th>';
                        $info->{object_val} .=    '<th colspan=2>Opers</th>';
                        $info->{object_val} .= '</tr>';
                        my ($style, $i);
                        my $style1  = "background-color: #f0f0f0;";
                        my $style2  = "background-color: #e0e0e0;";
                        my $neighs  = $cur_object->getNeighs();
                        foreach my $neigh (@$neighs){
                            my $neigh_id = $neigh->getId();
                            my $type     = $neigh->getType();
                            $style = (!$i%2) ? $style1 : $style2;
                            $info->{object_val} .= "<tr style=\"$style\""
                                ." class=\""
                                    ." neigh"
                                    ." object_id_$neigh_id"
                                    ." object_type_$type"
                                    ."\""
                                .">";
                            $info->{object_val} .= '<td><a'
                                .' href="/api?object_id='.$neigh_id.'"'
                                .'>'.$neigh->getNavName().'</a></td>';
                            $info->{object_val} .= "<td>$type</td>";
                            $info->{object_val} .= '<td><img'
                                .' class="bookmark_add"'
                                .' src="/static/images/16x16/plus1.png"'
                                .' alt="добавить в заметки"'
                                .'/></td>';
                            $info->{object_val} .= '<td><img'
                                .' class="neigh_unlink"'
                                .' src="/static/images/16x16/cut1.png"'
                                .'/></td>';
                            $info->{object_val} .= '<td><img'
                                .' class="neigh_delete"'
                                .' src="/static/images/16x16/del1.gif"'
                                .'/></td>';
                            $info->{object_val} .= "</tr>";
                            $i++;
                        }
                        $info->{object_val} .= '</table>';
                    } elsif ('descr' eq $object_id){
                        $info->{object_name} = 'Descr';
                        $info->{object_val}  = '<textarea name="descr" cols="40">'
                            .$info->{object_val}.'</textarea>';
                    }

                };
                my $ret = processProps(
                    objects   => ['_id', 'type', 'name', 'neighs', 'descr'],
                    cb      => $cb
                );
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

