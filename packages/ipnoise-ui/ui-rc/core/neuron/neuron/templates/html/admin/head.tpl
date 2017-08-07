<script>
    $(document).ready(function(){
        $('#updateElementary').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'descr':    $('#updateElementary').find('textarea[name="descr"]').val(),
                }
            );
            return false;
        });
    });
</script>

