<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateProduct').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'title':    $('#updateProduct').find('textarea[name="title"]').val()
                }
            });
            return false;
        });
    });
</script>

