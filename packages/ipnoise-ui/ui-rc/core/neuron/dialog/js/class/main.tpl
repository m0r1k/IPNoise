function NeuronDialogClass()
{
    var self = AbstractClass('NeuronClass');

    self.send = function(el, ev){
        var neuron_el   = $(el).parents('.neuron');
        var table_el    = $(neuron_el).find('.neuron_dialog_messages');
        var input_el    = $(neuron_el).find('.neuron_dialog_controls_input');
        var text        = input_el.val();
        var text_base64 = Base64.encode(text);
        var packet      = {
            'method':   'message_add',
            'params':   {
                'text_base64':   text_base64
            }
        };
        var args = {
            'packet':     packet, 
            'success':    function(a_data){
                var params      = a_data.params;
                var object_id   = params.object_id;

                // var neuron = NeuronDialogMsgClass();
                // neuron.setId(object_id);
                // neuron.load();
                // neuron.renderEmbeddedTo(table_el, 'widget');

                input_el.val('');
            }
        }
        self.sendApiPacket(args);
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

