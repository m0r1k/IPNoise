<div    class="IPNoiseObjectPhoto"
        object_id="<perl>return getCurObjectId()</perl>">
</div>

<!--
    <script>
        $(document).ready(function(){
            // setup object
            var object = ObjectPhotoClass();
            object.setId(
                "<perl>return getCurObjectId()</perl>"
            );
            object.load();
            object.render();
        });
    </script>

    <h1>Фото:
        <perl>
            my $ret = "\"".getCurObject()->getTitle()."\"";
            return $ret;
        </perl>
    </h1>

    <a href="/api?object_id=<perl>
            return getCurObject()->getId();
        </perl>&view=full"
        target="_blank">

        <img style="width: 600px" src="<perl>
            return getCurObject()->getUrl();
        </perl>"/>
    </a>

    <perl1>
        my $ret      = '';
        my $cur_user = getCurUser();
        if ($cur_user->isAdmin()){
            $ret .= processCurTemplate("upload.tpl");
        }
        return $ret;
    </perl1>
-->

