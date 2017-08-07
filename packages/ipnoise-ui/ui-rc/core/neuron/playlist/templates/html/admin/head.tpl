<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updatePlaylist').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'status':       $('#updatePlaylist').find('input[name="status"]').val(),
                    'play_order':   $('#updatePlaylist').find('input[name="play_order"]').val()
                }
            );
            return false;
        });
    });
</script>

