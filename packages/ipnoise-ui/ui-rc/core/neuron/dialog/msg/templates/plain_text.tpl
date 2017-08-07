<perl>
    my $ret     = '';
    my $neuron  = getCurNeuron();
    my $date    = $neuron->getDate();
    my $login   = $neuron->getUserLogin();
    my $text    = $neuron->getText();

    $ret .= "Date:  ".$date."\n";
    $ret .= "Login: ".$login."\n";
    $ret .= "Text:  ".$text."\n";

    return $ret;
</perl>

