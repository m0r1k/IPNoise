<perl>return processTemplate("core/object/object/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateUser').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'login':    $('#updateUser').find('input[name="login"]').val(),
                    'password': $('#updateUser').find('input[name="password"]').val()
                }
            });
            return false;
        });
    });
</script>

