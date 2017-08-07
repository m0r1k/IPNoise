<perl>
    my $ret         = '';
    my $cur_object  = getCurObject();
    my $title       = $cur_object->getTitle();
    my %args = (
        object_id   => $cur_object->getId(),
        title       => $title
    );
    $ret .= getHref(%args);

    return $ret;
</perl>

