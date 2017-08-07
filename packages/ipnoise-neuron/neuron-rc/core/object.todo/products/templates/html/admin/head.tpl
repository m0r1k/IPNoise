<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateProducts').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'title':    $('#updateProducts').find('textarea[name="title"]').val()
                }
            });
            return false;
        });
    });
</script>

