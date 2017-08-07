<h2>Разделы:</h2>
<ul>
<perl>
    my $ret     = '';
    my $neighs  = getCurNeighsByType(['products', 'shop']);
    foreach my $neigh (@$neighs){
        $ret .= '<li>';
        $ret .= $neigh->renderEmbedded('href');
        $ret .= '</li>';
    }
    return $ret;
</perl>
</ul>

