// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

/*
function ObjectDreamsClass()
{
    var self = AbstractClass('ObjectClass');

    self.showEditForm = function(a_args){
        var object_id = a_args.object_id;

        // setup object
        var object = ObjectDreamClass();
        object.setId(object_id);
        object.load();


        var new_dream_div = $(document).find('.object_dreams_add_new');
        new_dream_div.find('a').remove();

        object.renderEmbeddedTo(new_dream_div, 'edit');
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

                //var list    = $(document).find('.object_dreams_list');
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

