<script>
    $(document).ready(function(){
        $('#updateElementary').submit(function(e){
            e.preventDefault();
            updateObject({
                'object_id':    getCurObjectId(),
                'params':       {
                    'descr':    $('#updateElementary').find('textarea[name="descr"]').val(),
                }
            });
            return false;
        });
    });
</script>

