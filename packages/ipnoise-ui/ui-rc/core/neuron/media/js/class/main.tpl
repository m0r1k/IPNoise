$.widget("neurons.NeuronMedia", $.neurons.Neuron, {
    // default options
    options: {
        red:        255,
        green:      255,
        blue:       255,

        // TODO check what hashes works
        // here and use video: { ... }
        video_width:    800,
        video_height:   600,

        // neuron info
        neuron_status:                  'stop',
        neuron_player_tag:              '',
        neuron_show_controls_player:    0,
        neuron_show_controls_upload:    0,

        // cb
        play:   null
    },

    // the constructor
    _create: function(){
        this.element
        // add a class for theming
        .addClass("custom-colorize")
        // prevent double click to select text
        .disableSelection();

        this.option('have_render', 1);
        this._super();

        if (this.option('neuron_video_url')){
            this.option('neuron_player_tag', 'video');
            $('<'+this.option('neuron_player_tag')+'>', {
                'src':          this.options.neuron_url,
                'autobuffer':   'false',
                'width':        this.options.video_width,
                'height':       this.options.video_height
            }).appendTo(this.element);
        } else if (this.option('neuron_audio_url')){
            this.option('neuron_player_tag', 'audio');
            $('<'+this.option('neuron_player_tag')+'>', {
            }).appendTo(this.element);
            $('<source>', {
                'src':  this.options.neuron_url,
                'type': 'audio/mpeg'
            }).appendTo(
                $(this.element).find(
                    this.option('neuron_player_tag')
                )
            );
        }

        if (this.option('neuron_show_controls_player')){
            this.showPlayerControls();
        }
        if (this.option('neuron_show_controls_upload')){
            this.showUploadControls();
        }

        this._refresh();

        // init status
        var cur_status = this.option('neuron_status');
        this.processStatus(cur_status);
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        // remove generated elements
        this.changer.remove();

        this.element
            .enableSelection()
            .removeClass("custom-colorize");

        this._super();
    },

    _refresh: function(){
        this.element.css(
            "background-color",
            "rgb("
                +this.options.red   + ","
                +this.options.green + ","
                +this.options.blue
            +")"
        );
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

        // prevent invalid color values
        if (    'red'   == key
            ||  'green' == key
            ||  'blue'  == key)
        {
            if (val < 0 || val > 255){
                return;
            }
        }
        // may be super key
        this._super(key, val);
    },

    showPlayerControls: function(){
        var self = this;

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

        self.buttonsUpdate({
            'play':     'enabled',
            'pause':    'disabled',
            'stop':     'disabled'
        });

        self._on($(this.element).find('.play'), {
            // _on won't call when widget is disabled
            click: "play"
        });
        self._on($(this.element).find('.pause'), {
            // _on won't call when widget is disabled
            click: "pause"
        });
        self._on($(this.element).find('.stop'), {
            // _on won't call when widget is disabled
            click: "stop"
        });
    },

    showUploadControls: function(){
        $('<br>').appendTo(this.element);
        $('<br>').appendTo(this.element);
        $('<br>').appendTo(this.element);

        $('<form>', {
            'enctype':  'multipart/form-data',
            'method':   'post',
            'id':       'updatePhoto1',
            'action':   '/api?'
                +'object_id='+this.option('neuron_id')
                +'&method=upload'
        }).appendTo(this.element);

        var tr;
        var td;
        var input;
        var table;

        table = $('<table>').appendTo(
            $(this.element).find('form')
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
            $(this.element).find(
                this.option('neuron_player_tag')
            ).get(0).play();
        } else if ('pause' == a_status){
            self.buttonsUpdate({
                'play':     'enabled',
                'pause':    'disabled',
                'stop':     'enabled'
            });
            $(this.element).find(
                this.option('neuron_player_tag')
            ).get(0).pause();
        } else if ('stop' == a_status){
            self.buttonsUpdate({
                'play':     'enabled',
                'pause':    'disabled',
                'stop':     'disabled'
            });
            $(this.element).find(
                this.option('neuron_player_tag')
            ).get(0).load();
        } else {
            this.perror("unsupported status: '"+a_status+"'");
        }
    },

    apiEvent: function(a_event){
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        perror("TIGRA api event,"
            +" type: '"      + type      + "',"
            +" object_id: '" + object_id + "'"
        );

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
                button.prop('disabled', disabled);
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
                perror('play failed');
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
                perror('play failed');
            }
        };
        this.sendApiPacket(args);
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
                perror('play failed');
            }
        };
        this.sendApiPacket(args);
    },

});


