function NeuronProductClass()
{
    var self = AbstractClass('NeuronClass');

    self.addToBasket = function(){
        var params = {};
        var packet = {
            'method':   'addToBasket',
            'params':   params
        };
        var args = {
            packet:     packet,   
            success:    function(data){},
            failed:     function(data){}
        }
        self.sendApiPacket(args);
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

