$.widget("neurons.NeuronConsole", $.neurons.Neuron, {
    // default options
    options: {
    },

    // the constructor
    _create: function(){
        this.element
        // prevent double click to select text
        .disableSelection();

        this.option('have_render', 0);
        this._super();

        this._refresh();
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        this._super();
    },

    _refresh: function(){
        this._super();
    },

    // _setOptions is called with a hash
    // of all options that are changing
    // always refresh when changing options
    _setOptions: function(){
        // _super and _superApply handle keeping
        // the right this-context
        this._superApply(arguments);
        this._refresh();
    },

    // _setOption is called for each individual option
    // that is changing
    _setOption: function(key, val){
        // may be super key
        this._super(key, val);
    },

    apiEvent: function(a_event){
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        perror("MORIK api event for console,"
            +" type: '"      + type      + "',"
            +" object_id: '" + object_id + "'"
        );

        if ('props.changed' == type){
            var props = a_event.props;
            for (var prop_name in props){
                var prop_val = props[prop_name];
            }
        }

        // may be super event?
        this._super(a_event);
    },

});

