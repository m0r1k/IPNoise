<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateLink').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'index':    $('#updateLink').find('input[name="index"]').val()
                }
            );
            return false;
        });
    });
</script>

