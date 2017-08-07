// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

/*
function ObjectDialogClass()
{
    var self = AbstractClass('ObjectClass');

    self.send = function(el, ev){
        var object_el   = $(el).parents('.object');
        var table_el    = $(object_el).find('.object_dialog_messages');
        var input_el    = $(object_el).find('.object_dialog_controls_input');
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

                // var object = ObjectDialogMsgClass();
                // object.setId(object_id);
                // object.load();
                // object.renderEmbeddedTo(table_el, 'widget');

                input_el.val('');
            }
        }
        self.sendApiPacket(args);
    }

    // DO NOT FORGET RETURN SELF
    return self;
}
*/

