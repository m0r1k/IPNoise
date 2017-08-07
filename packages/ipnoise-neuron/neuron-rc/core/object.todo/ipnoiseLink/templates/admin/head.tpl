<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateLink').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'index':    $('#updateLink').find('input[name="index"]').val()
                }
            });
            return false;
        });
    });
</script>

