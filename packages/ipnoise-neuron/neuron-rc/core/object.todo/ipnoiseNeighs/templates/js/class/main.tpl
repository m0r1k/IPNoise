// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

/*
function ObjectIPNoiseNeighsClass()
{
    var self = AbstractClass('ObjectClass');

    self.showEditForm = function(a_args){
        var object_id = a_args.object_id;

        // setup object
        var object = ObjectIPNoiseNeighClass();
        object.setId(object_id);
        object.load();


        var new_ipnoise_neigh_div = $(document).find('.object_ipnoise_neighs_add_new');
        new_ipnoise_neigh_div.find('a').remove();

        object.renderEmbeddedTo(new_ipnoise_neigh_div, 'edit');
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

                //var list    = $(document).find('.object_ipnoise_neighs_list');
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
*/

