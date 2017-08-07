function NeuronIPNoiseNeighsClass()
{
    var self = AbstractClass('NeuronClass');

    self.showEditForm = function(a_args){
        var object_id = a_args.object_id;

        // setup neuron
        var neuron = NeuronIPNoiseNeighClass();
        neuron.setId(object_id);
        neuron.load();


        var new_ipnoise_neigh_div = $(document).find('.neuron_ipnoise_neighs_add_new');
        new_ipnoise_neigh_div.find('a').remove();

        neuron.renderEmbeddedTo(new_ipnoise_neigh_div, 'edit');
    }

    self.addNewIPNoiseNeigh = function(){
        var packet = {
            'method':   'add_new_ipnoise_neigh'
        };
        var args = {
            packet:     packet,
            success:    function(a_data){
                var params = a_data.params;
                self.showEditForm(params);

                //var list    = $(document).find('.neuron_ipnoise_neighs_list');
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

