<tr>
    <td>
    <perl>
        my $cur_object  = getCurObject();
        my $ret         = '';
        my $neighs      = getCurNeighsByType("photo");
        foreach my $neigh (@$neighs){
            $ret .= getHref(
                object_id   =>  $cur_object->getId(),
                title       =>  $neigh->renderEmbedded("small")
            );
            last;
        }
        return $ret;
    </perl>
    </td>
    <td>
    <perl>
        my $cur_object  = getCurObject();
        my $ret         = '';
        $ret .= getHref(
            object_id   => $cur_object->getId(),
            title       => $cur_object->getTitle()
        );
        return $ret;
    </perl>
    </td>
</tr>
