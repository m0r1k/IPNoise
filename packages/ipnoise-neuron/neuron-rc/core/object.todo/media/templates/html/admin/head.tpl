<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updatePhoto').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'title':    $('#updatePhoto').find('textarea[name="title"]').val(),
                    'url':      $('#updatePhoto').find('input[name="url"]').val()
                }
            });
            return false;
        });
    });
</script>

