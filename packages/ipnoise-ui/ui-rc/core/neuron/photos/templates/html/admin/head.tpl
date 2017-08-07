<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updatePhotos').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'title':    $('#updatePhotos').find('textarea[name="title"]').val(),
                }
            );
            return false;
        });
    });
</script>

