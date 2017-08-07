<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object objectOrder">

<script>
</script>

<tr>
    <td>2013.09.10</td>
    <td>12</td>
    <td>8910.20</td>
    <td>выполняется</td>
    <td>
        <perl>
            my $ret     = '';
            my $object  = getCurObject();
            $ret .= getHref(
                object_id   =>  $object->getId(),
                title       =>  'подробнее'
            );
        </perl>
    </td>
</tr>

</div>

