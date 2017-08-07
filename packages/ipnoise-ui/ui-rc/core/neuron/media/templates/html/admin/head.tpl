<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updatePhoto').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'title':    $('#updatePhoto').find('textarea[name="title"]').val(),
                    'url':      $('#updatePhoto').find('input[name="url"]').val()
                }
            );
            return false;
        });
    });
</script>

