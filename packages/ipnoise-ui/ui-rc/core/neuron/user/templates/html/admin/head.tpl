<perl>return processTemplate("core/neuron/neuron/templates/html/admin/head.tpl");</perl>
<script>
    $(document).ready(function(){
        $('#updateUser').submit(function(e){
            e.preventDefault();
            updateObject(
                getCurNeuronId(),
                {
                    'login':    $('#updateUser').find('input[name="login"]').val(),
                    'password': $('#updateUser').find('input[name="password"]').val()
                }
            );
            return false;
        });
    });
</script>

