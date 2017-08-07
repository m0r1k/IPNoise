<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>

<link rel="stylesheet" href="/static/jquery-ui.css" />
<link rel="stylesheet" href="/static/style.css"     />
<link rel="stylesheet" href="/static/table.css"     />

<script src="/static/js/jquery-1.9.1.js"                type="text/javascript" language="javascript"></script>
<script src="/static/js/jquery-ui.js"                   type="text/javascript" language="javascript"></script>
<script src="/static/js/jquery.MultiFile.js"            type="text/javascript" language="javascript"></script>
<script src="/static/js/jquery.cookie.js"               type="text/javascript" language="javascript"></script>

<script src="/static/js/AbstractClass.js"               type="text/javascript" language="javascript"></script>

<perl>
    my $ret = '';
    $ret .= '<script>';
    $ret .= processTemplate("js/class/all.tpl");
    $ret .= '</script>';
    return $ret;
</perl>

