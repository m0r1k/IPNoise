<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateProduct').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'title':    $('#updateProduct').find('textarea[name="title"]').val()
                }
            );
            return false;
        });
    });
</script>

