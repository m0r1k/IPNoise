// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseHtmlObjectMedia",
    $.ipnoise.IPNoiseHtmlObject,
{
    // default options
    options: {
        video_width:    800,
        video_height:   600,

        // object info
        object_status:                  'stop',
        object_player_tag:              '',
        object_show_controls_player:    0,
        object_show_controls_upload:    0,

        // cb
        play:   undefined,
    },

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseHtmlObjectMedia');

        if (self.option('object_video_url')){
            self.option('object_player_tag', 'video');
            $('<'+self.option('object_player_tag')+'>', {
                'src':          self.options.object_url,
                'autobuffer':   'false',
                'width':        self.options.video_width,
                'height':       self.options.video_height
            }).appendTo(self.element);
        } else if (self.option('object_audio_url')){
            self.option('object_player_tag', 'audio');
            $('<'+self.option('object_player_tag')+'>', {
            }).appendTo(self.element);
            $('<source>', {
                'src':  self.options.object_url,
                'type': 'audio/mpeg'
            }).appendTo(
                $(self.element).find(
                    self.option('object_player_tag')
                )
            );
        }

        if (self.option('object_show_controls_player')){
            self.showPlayerControls();
        }
        if (self.option('object_show_controls_upload')){
            self.showUploadControls();
        }

        self._refresh();

        // init status
        var cur_status = self.option('object_status');
        self.processStatus(cur_status);

        // do not forget return self!
        return self;
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;
        // remove generated elements
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

        self.buttonsUpdate({
            'play':     'enabled',
            'pause':    'disabled',
            'stop':     'disabled'
        });

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

    showUploadControls: function(){
        var self = this;

        $('<br>').appendTo(self.element);
        $('<br>').appendTo(self.element);
        $('<br>').appendTo(self.element);

        $('<form>', {
            'enctype':  'multipart/form-data',
            'method':   'post',
            'id':       'updatePhoto1',
            'action':   '/api?'
                +'object_id='+self.option('object_id')
                +'&method=upload'
        }).appendTo(self.element);

        var tr;
        var td;
        var input;
        var table;

        table = $('<table>').appendTo(
            $(self.element).find('form')
        );

        // first row
        tr = $('<tr>').appendTo(table);

        td = $('<td>').appendTo(tr);
        td.html("Файл для загрузки:");

        td    = $('<td>').appendTo(tr);
        input = $('<input>', {
            'type':   'file',
            'name':   'file',
            'tize':   '32',
            'class':  'multi max-1'
        }).appendTo(td);

        // second row
        tr      = $('<tr>').appendTo(table);
        td      = $('<td>').appendTo(tr);
        input   = $('<input>', {
            'type':   'submit',
            'value':  'Сохранить'
        }).appendTo(td);
    },

    processStatus: function(a_status){
        var self = this;

        if ('play' == a_status){
            self.buttonsUpdate({
                'play':     'disabled',
                'pause':    'enabled',
                'stop':     'enabled'
            });
            if (self.isWebGl()){
                self.initTextures(); // MORIK
            } else {
                $(self.element).find(
                    self.option('object_player_tag')
                ).get(0).play();
            }
        } else if ('pause' == a_status){
            self.buttonsUpdate({
                'play':     'enabled',
                'pause':    'disabled',
                'stop':     'enabled'
            });
            $(self.element).find(
                self.option('object_player_tag')
            ).get(0).pause();
        } else if ('stop' == a_status){
            self.buttonsUpdate({
                'play':     'enabled',
                'pause':    'disabled',
                'stop':     'disabled'
            });
            if (self.isWebGl()){
                self.initTextures2(); // MORIK
            } else {
                $(self.element).find(
                    self.option('object_player_tag')
                ).get(0).load();
            }
        } else {
            self.getLogger().perror(
                "unsupported status: '"+a_status+"'"
            );
        }
    },

    play:   function(){
        var self = this;
        self.buttonsDisable();

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

        self.buttonsDisable();

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

        self.buttonsDisable();

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

    apiEvent: function(a_event){
        var self = this;

        var type        = a_event.type;
        var object_id   = a_event.object_id;

        self.getLogger().perror("TIGRA api event,"
            +" type: '"      + type      + "',"
            +" object_id: '" + object_id + "'"
        );

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

    buttonsUpdate: function(a_buttons_info){
        var self = this;
        var buttons_info = {
            'play':     '',
            'pause':    '',
            'stop':     ''
        };

        for (var key in a_buttons_info){
            buttons_info[key] = a_buttons_info[key];
        }

        for (var button_name in buttons_info){
            var val = buttons_info[button_name];
            if (    'enabled'   == val
                ||  'disabled'  == val)
            {
                var button = self.element.find(
                    '.'+button_name
                );
                var disabled = ('disabled' == val);
                button.object('disabled', disabled);
                if (disabled){
                    button.animate({
                            color:              '#000000',
                            // backgroundColor: '#000000'
                        },
                        1000
                    );
                } else {
                    button.animate({
                            color:              'red',
                            // backgroundColor: '#333'
                        },
                        500
                    );
                }
            }
        }
    },

    buttonsDisable: function(){
        var self = this;
        self.buttonsUpdate({
            'play':     'disabled',
            'pause':    'disabled',
            'stop':     'disabled'
        });
    },

});

