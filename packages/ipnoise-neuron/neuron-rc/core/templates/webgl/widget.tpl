<perl>
    my $ret         = '';
    my $view        = getContext()->{view};

    my $cur_object  = getCurObject();
    $ret .= $cur_object->renderEmbedded($view);
    return $ret;
</perl>

