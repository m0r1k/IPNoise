<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>

<link rel="stylesheet" href="/static/jquery-ui.css" />
<link rel="stylesheet" href="/static/style.css"     />
<link rel="stylesheet" href="/static/table.css"     />

<script type="text/javascript" src="/static/js/jquery-1.9.1.js"     ></script>
<script type="text/javascript" src="/static/js/jquery-ui.js"        ></script>
<script type="text/javascript" src="/static/js/jquery.MultiFile.js" ></script>
<script type="text/javascript" src="/static/js/jquery.cookie.js"    ></script>

<script type="text/javascript">
<perl>
    my $ret = '';
    $ret .= processTemplate("js/constants.tpl");
    $ret .= processTemplate("js/log.tpl");
    $ret .= processTemplate("js/utils.tpl");
    $ret .= processTemplate("js/api.tpl");
    $ret .= processTemplate("js/class/all.tpl");
    $ret .= processTemplate("header/head.tpl");
    $ret .= processTemplate("navigation/head.tpl");
    $ret .= processTemplate("footer/head.tpl");
    return $ret;
</perl>
</script>

