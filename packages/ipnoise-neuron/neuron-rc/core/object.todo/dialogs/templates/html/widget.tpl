<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object objectDialogs">

    <script>
    </script>

    <div class="object_dialogs_add_new">
        <a href="" onclick="getCurObject().openNewDialog();return false;">Создать новый диалог</a>
    </div>

    <perl>
        my $ret = '';
        $ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

