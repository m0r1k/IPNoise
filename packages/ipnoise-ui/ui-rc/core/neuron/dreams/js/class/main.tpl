function NeuronDreamsClass()
{
    var self = AbstractClass('NeuronClass');

    self.showEditForm = function(a_args){
        var object_id = a_args.object_id;

        // setup neuron
        var neuron = NeuronDreamClass();
        neuron.setId(object_id);
        neuron.load();


        var new_dream_div = $(document).find('.neuron_dreams_add_new');
        new_dream_div.find('a').remove();

        neuron.renderEmbeddedTo(new_dream_div, 'edit');
    }

    self.addNewDream = function(){
        var packet = {
            'method':   'add_new_dream'
        };
        var args = {
            packet:     packet,
            success:    function(a_data){
                var params = a_data.params;
                self.showEditForm(params);

                //var list    = $(document).find('.neuron_dreams_list');
                //var parent  = list.parent();
                //list.remove();
                //self.renderEmbeddedTo(parent, 'list');
            },
        }
        self.sendApiPacket(args);
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

