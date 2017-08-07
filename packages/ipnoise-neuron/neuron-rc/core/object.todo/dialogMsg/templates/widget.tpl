<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object objectDialogMsg">

    <script>
    </script>

    <table class="object_dialog_msg">
        <tr>
            <perl>
                my $ret     = '';
                my $object  = getCurObject();
                my $date    = $object->getDate();
                my $login   = $object->getUserLogin();
                my $text    = $object->getText();

                $ret .= '<td>'.$date.'</td>';
                $ret .= '<td>'.$login.'</td>';
                $ret .= '<td class="object_dialog_msg_text">'.$text.'</td>';

                return $ret;
            </perl>
        </tr>
    </table>
</div>

