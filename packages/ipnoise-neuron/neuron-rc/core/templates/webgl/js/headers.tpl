<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>

<script type="text/javascript" src="static/webgl/lib/webgl-utils.js"    ></script>
<script type="text/javascript" src="/static/js/glMatrix-1.2.3.min.js"   ></script>
<script type="text/javascript" src="/static/js/jquery-1.9.1.js"         ></script>
<script type="text/javascript" src="/static/js/jquery-ui.js"            ></script>

<script type="text/javascript">
<perl>
    my $ret = '';
    $ret .= processTemplate("js/constants.tpl");
    $ret .= processTemplate("js/log.tpl");
    $ret .= processTemplate("js/utils.tpl");
    $ret .= processTemplate("js/api.tpl");
    $ret .= processTemplate("js/class/all.tpl");
    return $ret;
</perl>
</script>

<perl>
    my $ret = '';
    $ret .= processTemplate("js/widget_header.tpl");
    return $ret;
</perl>

