<!doctype html>
<html lang="en">
<head>

<perl>
    use core::object::object::main qw(processTemplate getCurUser);
    my $ret = '';
    $ret .= processTemplate("js/headers.tpl");
    $ret .= processTemplate("js/api.tpl");
    return $ret;
</perl>

</head>

<body>
    <h1>Page not found</h1>
    <perl>
        my $ret  = '';
        my $user = getCurUser();
        if (user){
            $ret .= "<a href=\"/api?object_id=";
            $ret .= $user->getId();
            $ret .= "\">";
            $ret .= "<h2>Go home</h2>";
            $ret .= "</a>";
        }
        return $ret;
    </perl>
</body>

</html>

