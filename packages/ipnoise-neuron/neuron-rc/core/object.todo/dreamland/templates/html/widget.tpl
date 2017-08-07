<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object">

    <script>
    </script>

    <table width="100%" class="object_dreamland">
        <tr>
            <td width="100%"><center><h1>Dreamland</h1></center></td>
            <td></td>
        </tr>
        <tr>
            <td class="object_dreamland_descr">
                <ul><li>
                    <perl>
                        my $ret     = '';
                        my $user    = getCurUser();
                        my $dreams  = $user->getNeighByType(
                            "core.object.dreams"
                        );
                        $ret .= getHref(
                            object_id   =>  $dreams->getId(),
                            title       =>  'открыть дневник'
                        );
                        return $ret;
                    </perl>
                </li></ul>
            </td>
        </tr>
    </table>
</div>

