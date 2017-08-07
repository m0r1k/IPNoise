function NeuronBasketClass()
{
    var self = AbstractClass('NeuronClass');

    self.makeOrder = function(){
        var packet = {
            'method':     'createOrder',
            'params':     {}
        };
        var args = {
            'packet':   packet,    
            'success':  function(data){
                var params      = data.params;
                var object_id   = params.object_id;
                gotoObject({
                    object_id:  object_id,
                    view:       'w1s1'
                });
            }
        };

        self.sendApiPacket(args);
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

