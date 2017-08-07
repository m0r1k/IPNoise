<!doctype html>
<html lang="en">
<head>

<perl>
    my $ret = '';
    $ret .= processTemplate("js/headers.tpl");
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

