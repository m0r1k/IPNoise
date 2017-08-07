<perl>
    my $ret = '';
    {
        my $session = getSession();
        if (!$session){
            next;
        }
        my $user = $session->getUser();
        if (!$user){
            next;
        }
        my $basket = $user->getBasket();
        if (!$basket){
            next;
        }

        $ret .= $basket->renderEmbedded('icon');
    }
</perl>

