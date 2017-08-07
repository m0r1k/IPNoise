<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateProducts').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'title':    $('#updateProducts').find('textarea[name="title"]').val()
                }
            );
            return false;
        });
    });
</script>

