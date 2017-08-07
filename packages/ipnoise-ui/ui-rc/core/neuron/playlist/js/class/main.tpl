$.widget("neurons.NeuronPlaylist", $.neurons.Neuron, {
    // default options
    options: {
        // neuron info
        neuron_status:      'stop',
        neuron_play_order:  'listen_time',

        // cb
        play:   null
    },

    // the constructor
    _create: function(){
        this.element
        // prevent double click to select text
        .disableSelection();

        this.option('have_render', 1);
        this._super();

        this.showPlaylist();
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        // remove generated elements
        this.changer.remove();

        this.element
            .enableSelection();

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

        console.log('key: '+key+' val: '+val);

        // may be super key
        this._super(key, val);
    },

    showPlaylist: function(){
        $('<table>', {
            'class':    'playlist'
        }).appendTo(this.element);

        $('<br>').appendTo(this.element);
        $('<br>').appendTo(this.element);

        // get media
        var packet = {
            'method':   'getNeighs',
            'params':   {
                'type':     'core.neuron.media'
            }
        };
        var self = this;
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params  = data.params;
                var neighs  = params.neighs;

                for (var i = 0; i < neighs.length; i++){
                    var neigh_id = neighs[i];
                    var tr = $('<tr>');
                    var td = $('<td>');
                    td.appendTo(tr);
                    render({
                        'object_id':    neigh_id,
                        'el':           td
                    });
                    $(self.element)
                        .find('.playlist')
                        .append(tr);
                }

                self.showPlayerControls();
                self._refresh();

                // init status
                var cur_status = self.option('neuron_status');
                self.processStatus(cur_status);
            },
            'failed':   function(data){
                perror('play failed');
            }
        };
        this.sendApiPacket(args);
    },

    showPlayerControls: function(){
        $('<br>').appendTo(this.element);

        $('<button>', {
            text:   'play',
            class:  'play'
        }).appendTo(this.element).button();

        $('<button>', {
            text:   'pause',
            class:  'pause'
        }).appendTo(this.element).button();

        $('<button>', {
            text:   'stop',
            class:  'stop'
        }).appendTo(this.element).button();


        this._on($(this.element).find('.play'), {
            // _on won't call when widget is disabled
            click: "play"
        });
        this._on($(this.element).find('.pause'), {
            // _on won't call when widget is disabled
            click: "pause"
        });
        this._on($(this.element).find('.stop'), {
            // _on won't call when widget is disabled
            click: "stop"
        });
    },

    processStatus: function(a_status){
        if ('play' == a_status){
            var media;
            for (var i = 0; i < 2; i++){
                media = $(this.element)
                    .find('.NeuronMedia')
                    .not('.was_played')
                    .first();
                if (media.length){
                    break;
                }
                // start plat all again
                $(this.element)
                    .find('.NeuronMedia')
                    .removeClass('was_played');
            } 
            if (media.length){
                media.addClass('was_played');
                media.NeuronMedia('play');
            }
        } else if ('pause' == a_status){
            $(this.element)
                .find('.NeuronMedia', 'pause');
        } else if ('stop' == a_status){
            $(this.element)
                .find('.NeuronMedia', 'load');

        } else {
            this.perror("unsupported status: '"+a_status+"'");
        }
    },

    apiEvent: function(a_event){
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        if ('props.changed' == type){
            var props = a_event.props;
            for (var prop_name in props){
                var prop_val = props[prop_name];
                if ('status' == prop_name){
                    // remove from event
                    delete props[prop_name];
                    this.processStatus(prop_val);
                }
            }
        }

        // may be super event?
        this._super(a_event);
    },

    play:   function(){
        var packet = {
            'method':     'setStatus',
            'params':     {
                'status':   'play'
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
            },
            'failed':   function(data){
                perror('play failed');
            }
        };
        this.sendApiPacket(args);
    },
    pause:  function(){
        var packet = {
            'method':     'setStatus',
            'params':     {
                'status':   'pause'
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
            },
            'failed':   function(data){
                perror('play failed');
            }
        };
        this.sendApiPacket(args);
    },
    stop:  function(){
        var packet = {
            'method':     'setStatus',
            'params':     {
                'status':   'stop'
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
            },
            'failed':   function(data){
                perror('play failed');
            }
        };
        this.sendApiPacket(args);
    },

});


