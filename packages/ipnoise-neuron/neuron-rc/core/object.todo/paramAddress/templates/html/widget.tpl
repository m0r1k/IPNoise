<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object">

    <script>
        $(document).ready(function(){
            // setup address
            var address = ObjectParamAddressClass();
            address.setId("<perl>return getCurObjectId()</perl>");
            address.load();

            <perl>
                my $ret = '';

                ## setup levels
                my $code    = "var levels = [\n";
                my $hash    = "";
                foreach my $level (@{getCurObject()->getLevels()}){
                    my $name        = $level->{name}        || '';
                    my $val         = $level->{val}         || '';
                    my $type_name   = $level->{type_name}   || '';
                    my $type_val    = $level->{type_val}    || '';
                    if ($hash){
                        chomp($hash);
                        $hash .= ",\n";
                    }
                    $hash .= "        {\n";
                    $hash .= "            name:         '$name',\n";
                    $hash .= "            type_name:    '$type_name',\n";
                    $hash .= "            type_val:     '$type_val',\n";
                    $hash .= "            val:          '$val'\n";
                    $hash .= "        }\n";
                }
                $code .= $hash;
                $code .= "    ];\n";
                $code .= "address.setLevels(levels);\n";
                $ret .= $code;

                return $ret;
            </perl>

            address.render();
        });
    </script>

    <table width="100%">
        <tr width="100%">
            <td width="100%"></td>
            <td><img src="/static/images/16x16/del1.gif"
                onclick="delObject($(this));"></td>
        </tr>
        <tr>
            <td colspan="1">
                <div class="body">
                    <table></table>
                </div>
            </td>
        </tr>
    </table>
    <hr/>
</div>

