<!doctype html>
<html lang="en">
<head>

<perl>
    my $ret = '';
    $ret .= processTemplate("js/headers.tpl");
    $ret .= processTemplate("admin/head.tpl");
    return $ret;
</perl>

</head>

<body>
    <div class="IPNoiseWebglViewer"></div>
</body>

</html>

