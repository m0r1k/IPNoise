<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateParam').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'value':    $('#updateParam').find('input[name="value"]').val()
                }
            });
            return false;
        });
    });
</script>

