<!doctype html>
<html lang="en">
<head>

<perl>
    my $ret = '';
    $ret .= processTemplate("headers.tpl");
    $ret .= processTemplate("utils.tpl");
    $ret .= processTemplate("api.tpl");
    $ret .= processTemplate("admin/head.tpl");
    $ret .= processTemplate("header/head.tpl");
    $ret .= processTemplate("navigation/head.tpl");
    $ret .= processTemplate("footer/head.tpl");
    return $ret;
</perl>

</head>

<!-- body -->

<body>

<table class="admin_page">
<tr>

<!-- col widget -->
<td class="admin_col_widget">
<perl>
    my $ret = '';
    $ret .= processTemplate("widget.tpl");
    return $ret;
</perl>
</td>

<!-- col admin -->
<td class="admin_col_admin">
<perl>
    my $ret = '';
    $ret .= processTemplate("admin/body.tpl");
    return $ret;
</perl>
</td>

</tr>
</table>

</body>
</html>

