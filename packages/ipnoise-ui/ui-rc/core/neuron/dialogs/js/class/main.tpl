function NeuronDialogsClass()
{
    var self = AbstractClass('NeuronClass');

    self.openNewDialog = function(){
        var packet = {
            'method':   'add_new_dialog'
        };
        var args = {
            'packet':   packet,   
            'success':  function(a_data){
                var params = a_data.params;
                gotoObject(params);
            }
        }
        self.sendApiPacket(args);
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

