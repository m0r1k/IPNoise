// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseHtmlObjectPlaylist",
    $.ipnoise.IPNoiseHtmlObject,
{
    // default options
    options: {
        // object info
        object_status:      'stop',
        object_play_order:  'listen_time',

        // cb
        play:   undefined
    },

    // the constructor
    _create: function(){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        self._super();

        self.showPlaylist();
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;

        // remove generated elements
        self.changer.remove();

        self.element.enableSelection();

        self._super();
    },

    _refresh: function(){
        var self = this;
        self._super();
    },

    // _setOptions is called with a hash
    // of all options that are changing
    // always refresh when changing options
    _setOptions: function(){
        var self = this;
        // _super and _superApply handle keeping
        // the right this-context
        self._superApply(arguments);
        self._refresh();
    },

    // _setOption is called for each individual option
    // that is changing
    _setOption: function(
        a_key,
        a_val,
        a_args)
    {
        var self = this;
        self._super(a_key, a_val, a_args);
    },

    showPlaylist: function(){
        var self = this;

        $('<table>', {
            'class':    'playlist'
        }).appendTo(self.element);

        $('<br>').appendTo(self.element);
        $('<br>').appendTo(self.element);

        // get media
        var packet = {
            'method':   'getNeighs',
            'params':   {
                'type':     'core.object.media'
            }
        };
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
                var cur_status = self.option('object_status');
                self.processStatus(cur_status);
            },
            'failed':   function(data){
                self.getLogger().perror('play failed');
            }
        };
        self.sendApiPacket(args);
    },

    showPlayerControls: function(){
        var self = this;
        $('<br>').appendTo(self.element);

        $('<button>', {
            text:   'play',
            class:  'play'
        }).appendTo(self.element).button();

        $('<button>', {
            text:   'pause',
            class:  'pause'
        }).appendTo(self.element).button();

        $('<button>', {
            text:   'stop',
            class:  'stop'
        }).appendTo(self.element).button();


        self._on($(self.element).find('.play'), {
            // _on won't call when widget is disabled
            click: "play"
        });
        self._on($(self.element).find('.pause'), {
            // _on won't call when widget is disabled
            click: "pause"
        });
        self._on($(self.element).find('.stop'), {
            // _on won't call when widget is disabled
            click: "stop"
        });
    },

    processStatus: function(a_status){
        var self = this;
        if ('play' == a_status){
            var media;
            for (var i = 0; i < 2; i++){
                media = $(self.element)
                    .find('.ObjectMedia')
                    .not('.was_played')
                    .first();
                if (media.length){
                    break;
                }
                // start plat all again
                $(self.element)
                    .find('.ObjectMedia')
                    .removeClass('was_played');
            } 
            if (media.length){
                media.addClass('was_played');
                media.ObjectMedia('play');
            }
        } else if ('pause' == a_status){
            $(self.element).find('.ObjectMedia', 'pause');
        } else if ('stop' == a_status){
            $(self.element).find('.ObjectMedia', 'load');
        } else {
            self.getLogger().perror(
                "unsupported status: '"+a_status+"'"
            );
        }
    },

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        if ('objects.changed' == type){
            var objects = a_event.objects;
            for (var object_name in objects){
                var object_val = objects[object_name];
                if ('status' == object_name){
                    // remove from event
                    delete objects[object_name];
                    self.processStatus(object_val);
                }
            }
        }

        // may be super event?
        self._super(a_event);
    },

    play:   function(){
        var self = this;

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
                self.getLogger().perror('play failed');
            }
        };
        self.sendApiPacket(args);
    },

    pause:  function(){
        var self = this;

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
                self.getLogger().perror('play failed');
            }
        };
        self.sendApiPacket(args);
    },

    stop:  function(){
        var self = this;

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
                self.getLogger().perror('play failed');
            }
        };
        self.sendApiPacket(args);
    },

});

