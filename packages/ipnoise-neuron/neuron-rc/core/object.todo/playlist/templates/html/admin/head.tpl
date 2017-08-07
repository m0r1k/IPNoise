<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updatePlaylist').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'status':       $('#updatePlaylist').find('input[name="status"]').val(),
                    'play_order':   $('#updatePlaylist').find('input[name="play_order"]').val()
                }
            );
            return false;
        });
    });
</script>

