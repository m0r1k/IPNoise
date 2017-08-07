$.widget("neurons.NeuronPhoto", $.neurons.Neuron, {
    // the constructor
    _create: function(){
        this._super();
    },

    // a public method to change the color to a random value
    // can be called directly via .colorize( "random" )
    random: function(event){
        var colors = {
            red:    Math.floor( Math.random() * 256 ),
            green:  Math.floor( Math.random() * 256 ),
            blue:   Math.floor( Math.random() * 256 )
        };

        updateObject(
            this.getNeuronId(),
            {
                descr: colors.red+':'+colors.green+':'+colors.blue
            }
        );
    }
});

