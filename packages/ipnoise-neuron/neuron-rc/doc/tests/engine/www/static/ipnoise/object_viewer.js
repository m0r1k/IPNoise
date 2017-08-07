$.widget('custom.IPNoiseObjectViewer', $.custom.IPNoiseObject, {
    // default options
    options: {
        'id':       undefined,
        'socket':   undefined,
        'type':     'ipnoise_object_viewer'
    },

    _create: function(){
        var self = this;

        self._super();

        var socket = self.option('socket');

        socket.on('addForDraw', function(a_action){
            var data = a_action.data;

            $('.viewer').empty();

            for (var i = 0; i < data.length; i++){
                var id = data[i];

                $('<div></div>')
                    .appendTo('.viewer')
                    .IPNoiseObject({ 'id': id });
            }
        });

        socket.on('actionRenderHtmlDone', function(a_action){
            // this - Socket
            var to      = a_action.to;
            var type    = a_action.type;

            //if (to != id){
            //    return;
            //}

            var self_el = $('#'+to);
            var self    = self_el.data(
                'custom-IPNoiseObject'
            );

            //if ('actionRender' == type){
            //    self.render();
            //} else if ('actionRenderHtml' == type){

            self.element.empty();
            try {
                eval(a_action.data);
            } catch (a_err){
                getLogger().pfatal(
                    "cannot render object: '"+to+"'",
                    a_err
                );
            }
            console.log(a_action.pos);
            if (a_action.pos){
                self.option('pos', a_action.pos);
            }

            //}
        });

    },

    emit: function(a_name, a_data){
        var self    = this;
        var socket  = self.option('socket');

        socket.emit(a_name, a_data);
    },

    render: function(){},

    on: function(a_name, a_cb){
        var self    = this;
        var socket  = self.option('socket');

        socket.on(a_name, a_cb);
    },

    close: function(){
        var self = this;
    }
});

