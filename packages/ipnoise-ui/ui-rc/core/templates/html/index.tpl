<!doctype html>
<html lang="en">
<head>

<perl>
    my $ret = '';
    $ret .= processTemplate("headers.tpl");
    $ret .= processTemplate("utils.tpl");
    $ret .= processTemplate("api.tpl");
    $ret .= processTemplate("header/head.tpl");
    $ret .= processTemplate("navigation/head.tpl");
    $ret .= processTemplate("footer/head.tpl");
    return $ret;
</perl>

</head>

<!-- body -->

<body>
<table class="page">
<tr>
<td>

<perl>
    my $ret = '';
    $ret .= processTemplate("widget.tpl");
    return $ret;
</perl>

</td>
</tr>
</table>

</body>
</html>

