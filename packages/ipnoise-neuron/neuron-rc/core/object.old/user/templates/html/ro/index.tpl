<!doctype html>
<html lang="en">
<head>

<perl>
    my $ret = '';
    $ret .= processTemplate("headers.tpl");
    $ret .= processTemplate("utils.tpl");
    $ret .= processTemplate("api.tpl");
    $ret .= processTemplate("admin/head.tpl");
    return $ret;
</perl>

<script>
    $(document).ready(function(){
        $('#tabs').tabs();
    });
</script>

</head>

<!-- body -->

<body>

<table class="page">
<tr>

<!-- col widget -->
<td class="page_widget">
<perl>
    my $cur_object = getCurObject();
    return $cur_object->renderEmbedded("ro/widget");
</perl>
</td>

<!-- col admin -->
<td class="page_admin">
<perl>
    return processTemplate("admin/body.tpl");
</perl>
</td>

</tr>
</table>

</body>
</html>

