<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object objectIPNoiseNeighs">

    <script>
    </script>

    <center>
    <h1>IPNoiseNeighs</h1>
    </center>

    <div class="object_ipnoise_neighs_add_new">
        <a href="" onclick="getCurObject().addNewNeigh();return false;">Добавить нового соседа</a>
    </div>

    <perl>
        my $ret = '';
        $ret = processCurTemplate('list.tpl');
        return $ret;
    </perl>

</div>

