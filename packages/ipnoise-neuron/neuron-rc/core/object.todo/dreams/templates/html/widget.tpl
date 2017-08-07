<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object objectDreams">

    <script>
    </script>

    <center>
    <h1>Dreams</h1>
    </center>

    <div class="object_dreams_add_new">
        <a href="" onclick="getCurObject().addNewDream();return false;">Добавить новый сон</a>
    </div>

    <perl>
        my $ret = '';
        $ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

