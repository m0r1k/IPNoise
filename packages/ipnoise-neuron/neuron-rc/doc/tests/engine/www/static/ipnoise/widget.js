$.widget('custom.IPNoiseWidget', $.Widget, {
    // default options
    options: {},

    _create: function(){
        var self = this;

        self.element.addClass('ipnoise_widget');
        self._super();
    },

    _destroy: function(){
        var self = this;
        self._super();
    },

    _setOption: function(a_key, a_val){
        var self = this;
        self._super(a_key, a_val);
    },

    on_action: function(a_socket, a_action){
        var self = this;
        var type = a_action.type;
    },

    getObject: function(){
        var self = this;

        var object_el = self.element.closest('.ipnoise_object');
        var object    = object_el.data('custom-IPNoiseObject');

        return object;
    },

    getObjectId: function(){
        var self = this;

        var object    = self.getObject();
        var object_id = object.option('id');

        return object_id;
    },

    emit: function(a_type, a_args){
        var self    = this;
        var object  = self.getObject();

        object.emit(a_type, a_args);
    },

    perror: function(a_msg, a_data){
        var self    = this;
        var object  = self.getObject();

        object.perror(a_msg, a_data);
    },

    getDocument: function(){
        var self    = this;
        var object  = self.getObject();
        var doc     = object.getDocument();

        return doc;
    }
});

