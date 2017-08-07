<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateParam').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'value':    $('#updateParam').find('input[name="value"]').val()
                }
            );
            return false;
        });
    });
</script>

