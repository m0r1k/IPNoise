<perl>
    my $ret     = '';
    my $object  = getCurObject();
    my $date    = $object->getDate();
    my $login   = $object->getUserLogin();
    my $text    = $object->getText();

    $ret .= "Date:  ".$date."\n";
    $ret .= "Login: ".$login."\n";
    $ret .= "Text:  ".$text."\n";

    return $ret;
</perl>

