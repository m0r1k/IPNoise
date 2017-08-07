<h3>Elementary</h3>
<div>
    <form metod="post" id="updateElementary">
        <table>
            <perl>
                my $cb = sub {
                    my $info = shift;

                    my $cur_neuron  = getCurNeuron();
                    my $type        = $cur_neuron->getType();
                    my $prop_id     = $info->{prop_id};

                    if ('_id' eq $prop_id){
                        $info->{prop_name} = "ID";
                        $info->{prop_val}  = '<input'
                            .' type="text" size="40"'
                            .' name="id"'
                            .' style="background-color: #e0e0e0;" readonly'
                            .' value="'.$cur_neuron->getId().'">'
                            .'</input>';
                    } elsif ('type' eq $prop_id){
                        $info->{prop_id}   = 'Type';
                    } elsif ('name' eq $prop_id){
                        $info->{prop_name} = "Name";
                        $info->{prop_val}  = '<input'
                            .' type="text" size="40"'
                            .' name="name"'
                            .' style="background-color: #e0e0e0;"'
                            .' value="'.$cur_neuron->getName().'">'
                            .'</input>';
                    } elsif ('neighs' eq $prop_id){
                        $info->{prop_name} = 'Neighs';
                        $info->{prop_val}  = '<table'
                            .' style="background-color: #c0c0c0"'
                            .' width="100%"'
                            .'>';
                        $info->{prop_val} .= '<tr>';
                        $info->{prop_val} .=    '<th style="width: 100%">ID</th>';
                        $info->{prop_val} .=    '<th>Type</th>';
                        $info->{prop_val} .=    '<th colspan=2>Opers</th>';
                        $info->{prop_val} .= '</tr>';
                        my ($style, $i);
                        my $style1  = "background-color: #f0f0f0;";
                        my $style2  = "background-color: #e0e0e0;";
                        my $neighs  = $cur_neuron->getNeighs();
                        foreach my $neigh (@$neighs){
                            my $neigh_id = $neigh->getId();
                            my $type     = $neigh->getType();
                            $style = (!$i%2) ? $style1 : $style2;
                            $info->{prop_val} .= "<tr style=\"$style\""
                                ." class=\""
                                    ." neigh"
                                    ." object_id_$neigh_id"
                                    ." object_type_$type"
                                    ."\""
                                .">";
                            $info->{prop_val} .= '<td><a'
                                .' href="/api?object_id='.$neigh_id.'"'
                                .'>'.$neigh->getNavName().'</a></td>';
                            $info->{prop_val} .= "<td>$type</td>";
                            $info->{prop_val} .= '<td><img'
                                .' class="bookmark_add"'
                                .' src="/static/images/16x16/plus1.png"'
                                .' alt="добавить в заметки"'
                                .'/></td>';
                            $info->{prop_val} .= '<td><img'
                                .' class="neigh_unlink"'
                                .' src="/static/images/16x16/cut1.png"'
                                .'/></td>';
                            $info->{prop_val} .= '<td><img'
                                .' class="neigh_delete"'
                                .' src="/static/images/16x16/del1.gif"'
                                .'/></td>';
                            $info->{prop_val} .= "</tr>";
                            $i++;
                        }
                        $info->{prop_val} .= '</table>';
                    } elsif ('descr' eq $prop_id){
                        $info->{prop_name} = 'Descr';
                        $info->{prop_val}  = '<textarea name="descr" cols="40">'
                            .$info->{prop_val}.'</textarea>';
                    }

                };
                my $ret = processProps(
                    props   => ['_id', 'type', 'name', 'neighs', 'descr'],
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

