// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseWebglObjectShop",
    $.ipnoise.IPNoiseWebglObject,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        if (undefined == a_args.viewer){
            getLogger().pfatal("missing argument: 'viewer'");
        }

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglObjectShop');

        self.m_size                 = 3;
        self.m_scale                = 0.85/self.m_size
        self.m_tweens               = [];
        self.m_cubeStates           = [];

        // do not forget return self!
        return self;
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;
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

    setPos: function(
        a_v3_pos)
    {
        var self = this;
        getLogger().pdebug(1, "morik setpos", a_v3_pos);
        self.m_pos = a_v3_pos;
    },

    scramble: function(scramble){
        var self = this;
        var now  = new Date().getTime();

        self.m_tweens.length = 0;

        if (scramble){
            var r = Math.sqrt(0.5);
            // Scramble several times, for style
            for (var j=0; j < 5; j++){
                // First make a list of possible locations
                var s = (self.m_size-1)/2;
                var spots = [];
                for (var z = 0; z < self.m_size; z++)
                for (var y = 0; y < self.m_size; y++)
                for (var x = 0; x < self.m_size; x++){
                    spots.push(
                        vec3.create([
                            (x-s)/self.m_size,
                            (y-s)/self.m_size,
                            (z-s)/self.m_size
                        ])
                    );
                }

                // Mix up the list
                spots.sort(function(){
                    return 0.5 - Math.random()
                });

                for (var i = 0;
                    i < self.m_cubeStates.length;
                    i++)
                {
                    // Select a random, orthogonal orientation
                    // This algorithm probably sucks,
                    // but it works.
                    var randRot = quat4.create([0,0,0,1]);
                    for (var k=0; k < 1; k++){
                        switch (Math.floor(Math.random()*3)){
                            case 0:
                                quat4.multiply(randRot, [r,0,0,r]);
                                break;

                            case 1:
                                quat4.multiply(randRot, [0,r,0,r]);
                                break;

                            case 2:
                                quat4.multiply(randRot, [0,0,r,r]);
                                break;
                        }
                    }

                    self.m_tweens.push({
                        type:       "move",
                        index:      i,
                        from:       self.m_cubeStates[i].pos,
                        to:         spots.pop(),
                        startTime:  now + j*500 + i*15,
                        duration:   500,
                    });

                    self.m_tweens.push({
                        type:       "rotate",
                        index:      i,
                        from:       self.m_cubeStates[i].rot,
                        to:         randRot,
                        startTime:  now + j*500 + i*10 + 300,
                        duration:   500,
                    });
                }
            }
        } else {
            for (var i = 0; i < self.m_cubeStates.length; i++){
                self.m_tweens.push({
                    type:           "move",
                    index:          i,
                    from:           self.m_cubeStates[i].pos,
                    to:             self.m_cubeStates[i].homePos,
                    startTime:      now,
                    duration:       1000,
                });

                self.m_tweens.push({
                    type:           "rotate",
                    index:          i,
                    from:           self.m_cubeStates[i].rot,
                    to:             [0,0,0,1],
                    startTime:      now,
                    duration:       1000,
                });
            }
        }
    },

    setGap: function(
        a_size)
    {
        var self = this;

        var newScale;

        switch (a_size){
            case 0:
                newScale = 1.00 / self.m_size;
                break;

            case 1:
                newScale = 0.85 / self.m_size;
                break;

            case 2:
                newScale = 0.65 / self.m_size;
                break;
        }

        self.m_tweens.push({
            type:       "scale",
            from:       self.scale,
            to:         newScale,
            startTime:  new Date().getTime(),
            duration:   1000,
        });
    },

    createCube: function(){
        var self    = this;

        var gl      = self.getGl();
        var size    = self.m_size;
        var s       = (size - 1)/2;
        var i       = 0;

        for (var z = 0; z < size; z++)
        for (var y = 0; y < size; y++)
        for (var x = 0; x < size; x++)
        {
            var mag = [
                x > 0        ? 1 : 0,
                x < size - 1 ? 1 : 0,
                y > 0        ? 1 : 0,
                y < size - 1 ? 1 : 0,
                z > 0        ? 1 : 0,
                z < size - 1 ? 1 : 0,
            ];

            var spots = [];
            for (j = 0; j < mag.length; j++){
                for (k = 0; k < 4; k++){
                    if (mag[j]){
                        spots.push(0.8, 0.8, 0.4, 1.0);
                    } else {
                        spots.push(0.0, 0.0, 0.0, 0.0);
                    }
                }
            }
            var spotBuf = gl.createBuffer();
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                spotBuf
            );
            gl.bufferData(
                gl.ARRAY_BUFFER,
                new Float32Array(spots),
                gl.STATIC_DRAW
            );

            i++;
            var pick_color = 0;
            pick_color = i * 256*256*256/(size*size*size - 1);

            self.getCreateNeigh({
                'params':   {
                    'name':     x+':'+y+':'+z,
                    'pos':      [
                        self.m_pos[X] + (x-s)/10,
                        self.m_pos[Y] + (y-s)/10,
                        self.m_pos[Z] + (z-s)/10
                    ],
                    'type':     'core.object.media'
                },
                'args':     {
                    'viewer':   self.element
                },
                'success':  function(a_data, a_widgets){
                    if (1 != a_widgets.length){
                        self.getLogger().pfatal(
                            "invalid widgets count",
                            {
                                'a_data':    a_data,
                                'a_widgets': a_widgets
                            }
                        );
                    }

                    var cub     = a_widgets[0];
                    var pos_str = cub.callWidgetMethod(
                        'option',
                        'object_pos'
                    );
                    var pos   = JSON.parse(pos_str);
                    var pos_x = pos[0];
                    var pos_y = pos[1];
                    var pos_z = pos[2];

                    var pos = vec3.create([
                        pos_x,
                        pos_y,
                        pos_z
                    ]);

                    self.getLogger().pdebug(
                        11,
                        "create media,"
                        +" size: '"+size+"',"
                        +" pos:",
                        pos
                    );
                    cub.callWidgetMethod(
                        'setPos',
                        pos
                    );
                    cub.callWidgetMethod(
                        'setPosHome',
                        vec3.create([
                            pos_x,
                            pos_y,
                            pos_z
                        ])
                    );
                    cub.callWidgetMethod(
                        'setRot',
                        quat4.create([
                            0, 0, 0, 1
                        ])
                    );
                    cub.callWidgetMethod(
                        'setTex',
                        vec3.create([
                            pos_x,
                            pos_y,
                            pos_z
                        ])
                    );
                    cub.callWidgetMethod(
                        'setMag',
                        mag
                    );
                    cub.callWidgetMethod(
                        'setSpot',
                        spotBuf
                    );
                    cub.callWidgetMethod(
                        'setScale',
                        self.m_scale
                    );
                    cub.callWidgetMethod(
                        'setSize',
                        1/self.m_size
                    );

                    self.m_cubeStates.push(cub);

                    self.m_viewer.callWidgetMethod(
                        'addForDraw',
                        cub
                    );
                }
            });
        }
    },

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        if ('objects.changed' == type){
            var objects = a_event.objects;
        }

        // may be super event?
        self._super(a_event);
    },

    // ---------------- api ----------------

    animate: function(){},

    draw: function(
        a_pick_color)
    {
        var self = this;
    },

    select:     function(){},
    deSelect:   function(){},

    // -------------------------------------
});

