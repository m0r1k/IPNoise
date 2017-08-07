<table width="100%" class="navigation">
<tr>
<td>
<perl>
    my $ret         = '';
    my $cur_object  = getCurObject();
    my $neighs      = $cur_object->getNeighs(
        type => [ 'shop', 'photo', 'products', 'product' ]
    );
    foreach my $neigh (@$neighs){
        if ($ret){
            $ret .= ", ";
        }
        my %args = (
            object_id   => $neigh->getId(),
            title       => $neigh->getNavName()
        );
        $ret .= getHref(%args);
    }
    return $ret;
</perl>
</td>
</tr>
</table>

