// vim:syntax=javascript tabstop=4 expandtab

// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

var objects_by_id = {};

// ---------------- object ----------------

$.widget(
    "ipnoise.IPNoiseObject",
{
    // default options
    options: {
        api_subscribe:  { 'all': 1 },

        // object
        object_id:      undefined,
        object_url:     undefined,

        // callbacks
        change:         undefined
    },

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseObject');

        // init options
        self._init_options();

        var api_subscribe = self.option('api_subscribe');
        if (undefined != api_subscribe){
            apiUpdateEventsSubscr({
                'object_id':    self.option('object_id'),
                'events':       api_subscribe
            });
        }

        // do not forget return self!
        return self;
    },

    // called when created, and later when changing options
    _refresh: function(){
        var self = this;
        // trigger a callback/event
        self._trigger( "change" );
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;
        self.getLogger().pwarn("TODO unsubscribe from events");
        self.element.css("background-color", "transparent");
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
        var self        = this;
        var object_id   = self.option('object_id');

        var args        = {
            'setup_attribute':  1,
            'handled':          0
        };
        copy_args(args, a_args);

        var setup_attribute = args.setup_attribute;
        if (setup_attribute){
            // setup attribute
            self.element.attr(a_key, a_val);
        }

        // Object
        if (    'object_id'             == a_key
            ||  'object_type'           == a_key
            ||  'object_removed'        == a_key
            ||  'object_name'           == a_key
            ||  'object_title'          == a_key
            ||  'object_descr'          == a_key
            ||  'object_url'            == a_key
            ||  'object_view_strafe'    == a_key
            ||  'object_view_ryp'       == a_key
            ||  'object_view_zoom'      == a_key)
        {
            // store object base objects
            args.handled = 1;
        } else if ( 'object_pos'        == a_key
            ||      'object_view_pos'   == a_key
            ||      'object_view_up'    == a_key)
        {
            args.handled = 1;
        } else if ( 'object_neighs'         == a_key
            ||      'object_cur_object_id'  == a_key
            ||      'object_events'         == a_key
            ||      'object_value'          == a_key)
        {
            // store others objects
            // TODO move to upper layer
            args.handled = 1;
        }

        if (args.handled){
            self._super(a_key, a_val);
        } else {
            self.getLogger().pfatal(
                "unsupported option",
                {
                    'a_key':    a_key,
                    'a_val':    a_val
                }
            );
        }

        if (object_id){
            // for 'edit' forms
            var objects = $("div[object_id='"+object_id+"']");
            objects.find("input[name='"+a_key+"']").val(a_val);
            objects.find("textarea[name='"+a_key+"']").val(a_val);
        }
    },

    // init element options from element attributes
    _init_options: function(){
        var self        = this;
        var attributes  = self.element.getAttributes();
        var object_id   = attributes.object_id;
        var object_type = attributes.object_type;

        // first of all setup type and ID
        self._setOption('object_id',   object_id,   { 'setup_attribute': 0 });
        self._setOption('object_type', object_type, { 'setup_attribute': 0 });

        for (var key in attributes){
            var opt_val = attributes[key];
            var res = /^(object_.+)/.exec(key);
            if (res){
                var opt_name = res[1];
                if ('string' != jQuery.type(opt_name)){
                    continue;
                }
                if (    'object_id'     == opt_name
                    ||  'object_type'   == opt_name)
                {
                    // already set above
                    continue;
                }
                self._setOption(opt_name, opt_val, { 'setup_attribute': 0 });
            }
        }
    },

    setLogger: function(
        a_logger)
    {
        var self = this;
        self.m_logger = a_logger;
    },

    getLogger: function(){
        var self = this;
        return self.m_logger;
    },

    strafe: function(a_val){
        var self = this;
        var args = {
            'params':   {
                'accel_strafe':  a_val
            },
            'success':  undefined,
            'failed':   undefined,
            'args':     {}
        };

        var params       = args.params;
        var accel_strafe = params.accel_strafe;

        if (!accel_strafe){
            getLogger().pfatal("missing param 'accel_strafe'");
        }

        var packet = {
            'method':       'accel',
            'params':       params,
            'success':      args.success,
            'failed':       args.failed
        };

        var data = {
            'packet':   packet,
        };
        self.sendApiPacket(data);
    },

    accel: function(a_args){
        var self        = this;
        var args        = {
            'accel':    1,
            'params':   {},
            'success':  undefined,
            'failed':   undefined,
            'args':     {}
        };

        // copy args
        copy_args(args, a_args);

        if (undefined == args.accel){
            self.getLogger().pfatal(
                "missing argument 'accel'"
            );
        }

        var pos      = JSON_STR_TO_VEC3(self.option('object_pos'));
        var view_pos = JSON_STR_TO_VEC3(self.option('object_view_pos'));

        var accel = [
            args.accel * (view_pos[X] - pos[X]),
            args.accel * (view_pos[Y] - pos[Y]),
            args.accel * (view_pos[Z] - pos[Z])
        ];

        var params   = args.params;
        params.accel = accel;

        var packet = {
            'method':       'accel',
            'params':       params,
            'success':      args.success,
            'failed':       args.failed
        };

        var data = {
            'packet':   packet,
        };
        self.sendApiPacket(data);
    },

    rotateViewVector: function(a_args){
        var self        = this;
        var args        = {
            'params':   {},
            'success':  undefined,
            'failed':   undefined,
            'args':     {}
        };

        // copy args
        copy_args(args, a_args);

        var params  = args.params;
        // yaw, pitch in radians
        var yaw     = params.yaw;
        var pitch   = params.pitch;

        if (    !yaw
            &&  !pitch)
        {
            self.getLogger().pfatal(
                "at least one param must be defined"
                +" 'yaw' or 'pitch'",
                args
            );
        }

        var packet = {
            'method':       'rotateViewVector',
            'params':       params,
            'success':      args.success,
            'failed':       args.failed
        };

        var data = {
            'packet':   packet,
        };
        self.sendApiPacket(data);
    },

    rotate_position: function(
        a_angle,
        a_x,
        a_y,
        a_z)
    {
        var self = this;

        self.m_pos[X] = self.m_pos[X] - self.m_pos_view[X];
        self.m_pos[Y] = self.m_pos[Y] - self.m_pos_view[Y];
        self.m_pos[Z] = self.m_pos[Z] - self.m_pos_view[Z];

        var v_vector = self.m_pos;
        var AVector = vec3.create();

        var SinA = Math.sin(Math.PI * a_angle / 180.0);
        var CosA = Math.cos(Math.PI * a_angle / 180.0);

        // Найдем новую позицию X для вращаемой точки
        AVector[X] = (CosA + (1 - CosA) * a_x * a_x) * v_vector[X];
        AVector[X] += ((1 - CosA) * a_x * a_y - a_z * SinA) * v_vector[Y];
        AVector[X] += ((1 - CosA) * a_x * a_z + a_y * SinA) * v_vector[Z];

        // Найдем позицию Y
        AVector[Y] = ((1 - CosA) * a_x * a_y + a_z * SinA) * v_vector[X];
        AVector[Y] += (CosA + (1 - CosA) * a_y * a_y) * v_vector[Y];
        AVector[Y] += ((1 - CosA) * a_y * a_z - a_x * SinA) * v_vector[Z];

        // И позицию Z
        AVector[Z] = ((1 - CosA) * a_x * a_z - a_y * SinA) * v_vector[X];
        AVector[Z] += ((1 - CosA) * a_y * a_z + a_x * SinA) * v_vector[Y];
        AVector[Z] += (CosA + (1 - CosA) * a_z * a_z) * v_vector[Z];

        self.m_pos[X] = self.m_pos_view[X] + AVector[X];
        self.m_pos[Y] = self.m_pos_view[Y] + AVector[Y];
        self.m_pos[Z] = self.m_pos_view[Z] + AVector[Z];

        self.getLogger().pdebug(50,
            "camera::rotate_position:\n"
            +" speed: '"+a_speed+"'\n"
            +" pos:",
            self.m_pos
        );
    },

    getCreateNeigh: function(a_args){
        var self = this;
        var args = {
            'params':   {
                'type': 'core.object.object'
            },
            'success':  undefined,
            'failed':   undefined,
            'args':     {}
        };

        // copy args
        for (var key in a_args){
            args[key] = a_args[key];
        }

        var params = args.params;

        var packet = {
            'method':       'getCreateNeigh',
            'params':       params,
            'success':      function(a_data){
                var params = a_data.params;
                if (params && params.html){
                    var widgets = parseHtmlOnWidgets({
                        'html_base64':  params.html,
                        'args':         args.args
                    });
                    if (args.success){
                        args.success(a_data, widgets);
                    }
                } else {
                    if (args.failed){
                        args.failed(a_data);
                    }
                }
            },
            'failed':       function(a_data){
                if (args.failed){
                    args.failed(a_data);
                }
            }
        };

        var data = {
            'packet':   packet,
        };
        self.sendApiPacket(data);
    },

    isWebGl: function(){
        var ret = 0;
        var res = getUrlParameter('webgl');
        if ('1' == res){
            ret = res;
        }
        return ret;
    },

    setPosHome: function(a_arg){
        var self        = this;
        self.m_pos_home = a_arg;
    },

    setRot: function(a_arg){
        var self    = this;
        self.m_rot  = a_arg;
    },

    setTex: function(a_arg){
        var self    = this;
        self.m_tex  = a_arg;
    },

    setMag: function(a_arg){
        var self    = this;
        self.m_mag  = a_arg;
    },

    setSpot: function(a_arg){
        var self    = this;
        self.m_spot = a_arg;
    },

    setScale: function(a_arg){
        var self        = this;
        self.m_scale    = a_arg;
    },

    setSize: function(a_arg){
        var self        = this;
        self.m_size     = a_arg;
    },

/*
    move: function(a_axis, a_dir){
        var self = this;

        //var axis = self.m_selectedFace >> 1;
        //var sign = (self.m_selectedFace & 1) ? -1 : 1;

        self.getLogger().pdebug(1, "move", {
            'axis': a_axis,
            //'sign': a_sign,
            'dir':  a_dir
        });

        // Is there already a tween for this?
        //var oldtw;
        //for (var i = 0; i < self.m_tweens.length; i++){
        //    if ("move" == self.m_tweens[i].type){
        //        oldtw = self.m_tweens.splice(i, 1)[0];
        //        break;
        //    }
        //}

        //var oldPos    = self.m_pos;
        var newPos      = [0,0,0];
        newPos[a_axis]  = a_dir;

        getLogger().pdebug(1, "MORIK new_pos", newPos);

        quat4.multiplyVec3(
            self.m_rot,
            newPos
        );

        //vec3.add(
        //    newPos,
        //    oldtw
        //        ? oldtw.to
        //        : oldPos
        //);

        self.updateObject({
            'params': {
                'pos_add':  newPos
            }
        });

//        self.m_tweens.push({
//            type:       "move",
//            from:       oldPos,
//            to:         newPos,
//            startTime:  new Date().getTime(),
//            duration:   10000,
//        });
    },
*/

    move: function(a_args){
        var self = this;

        var args = {
            'params':   {
                'object_id':    undefined
            }
        };
        copy_args(args, a_args);

        var object_id = args.params.object_id;

        if (undefined == object_id){
            self.getLogger().pfatal(
                "missing param: 'object_id'",
                args
            );
        }

        var packet  = {
            'method':       'move',
            'params':       args.params
        };
        var data = {
            'packet':   packet,
        };
        self.sendApiPacket(data);
    },

    rotate: function(dir){
        var self = this;

        var axis = self.m_selectedFace >> 1;
        var sign = ((self.m_selectedFace & 1) ? 1 : -1) * dir;

        // Is there already a tween for this?
        var oldtw;
        for (var i = 0; i < self.m_tweens.length; i++){
            if ("rotate" == self.m_tweens[i].type){
                oldtw = self.m_tweens.splice(i, 1)[0];
                break;
            }
        }

        var oldRot  = self.m_rot;
        var newRot  = quat4.create(oldtw?oldtw.to:oldRot);

        var rot     = [0,0,0,Math.sqrt(0.5)];
        rot[axis]   = sign*Math.sqrt(0.5);

        quat4.multiply(newRot, rot);

        self.m_tweens.push({
            type:       "rotate",
            from:       oldRot,
            to:         newRot,
            startTime:  new Date().getTime(),
            duration:   250,
        });
    },
 
    // send api
    sendApiPacket: function(a_args){
        var self = this;
        var args = {
            'packet':   undefined
        }

        // copy args
        for (var key in a_args){
            args[key] = a_args[key];
        }

        var packet = args.packet;
        if (undefined == packet){
            getLogger().pfatal("missing argument: 'packet'",
                args
            );
        }

        // setup id
        args.packet.object_id = self.option('object_id');

        sendApiPacket(args);
    },

    getObjectClassName: function(){
        var self = this;
        var ret  = getIPNoiseClassName(self.element);
        return ret;
    },

    updateObject: function(a_args){
        var self = this;
        var args = {
            'object_id':  self.option('object_id'),
            'params':     undefined,
            'success':    undefined,
            'failed':     undefined
        };

        // copy args
        for (var key in a_args){
            args[key] = a_args[key];
        }

        var params = args.params;

        if (undefined == params){
            self.getLogger().pfatal(
                "missing argument 'params'",
                args
            );
        }

        updateObject(args);
    },

    // ---------------- api ----------------

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        self.getLogger().pdebug(10, "api event", {
            'type':         type,
            'object_id':    object_id
        });

        if ('objects.changed' == type){
            var objects = a_event.objects;
            for (var object_name in objects){
                var object_val = objects[object_name];

                if ('descr' == object_name){
                    self.option('object_'+object_name, object_val);
                } else if ( 'pos'           == object_name
                    ||      'view_pos'      == object_name
                    ||      'view_up'       == object_name
                    ||      'view_strafe'   == object_name
                    ||      'view_ryp'      == object_name)
                {
                    var pos = '['
                        +object_val[0]+','
                        +object_val[1]+','
                        +object_val[2]
                        +']';
                    self.option('object_'+object_name, pos);
                } else if ( 'neighs'         == object_name
                    ||      'cur_object_id'  == object_name
                    ||      'events'         == object_name
                    ||      'value'          == object_name
                    ||      'view_zoom'      == object_name)
                {
                    // TODO move from here to upper layer
                    self.option('object_'+object_name, pos);
                } else {
                    self.getLogger().pfatal(
                        "Unsupported objecterty: '"+object_name+"'"
                        +" in event: '"+type+"'"
                    );
                }
            }
        } else {
            self.getLogger().pfatal(
                "Unsupported event: '"+type+"'"
            );
        }
    },

    handleKeys: function(
        a_cur_pressed_keys)
    {
        var self = this;

        var key_info_control = a_cur_pressed_keys[KeyEvent.DOM_VK_CONTROL];
        var key_info_right   = a_cur_pressed_keys[KeyEvent.DOM_VK_RIGHT];
        var key_info_left    = a_cur_pressed_keys[KeyEvent.DOM_VK_LEFT];
        var key_info_up      = a_cur_pressed_keys[KeyEvent.DOM_VK_UP];
        var key_info_down    = a_cur_pressed_keys[KeyEvent.DOM_VK_DOWN];
        var key_info_w       = a_cur_pressed_keys[KeyEvent.DOM_VK_W];
        var key_info_s       = a_cur_pressed_keys[KeyEvent.DOM_VK_S];
        var key_info_a       = a_cur_pressed_keys[KeyEvent.DOM_VK_A];
        var key_info_d       = a_cur_pressed_keys[KeyEvent.DOM_VK_D];
        // modes
        var key_info_p       = a_cur_pressed_keys[KeyEvent.DOM_VK_P];
        var key_info_f       = a_cur_pressed_keys[KeyEvent.DOM_VK_F];
        var key_info_c       = a_cur_pressed_keys[KeyEvent.DOM_VK_C];

        if (key_info_p){
            if (    undefined == key_info_p.inc.send_delay
                ||  key_info_p.inc.send_delay >= 100)
            {
                key_info_p.inc.send_delay = 0;
                self.m_show_pick_fbuffer_flag
                    = !self.m_show_pick_fbuffer_flag;
            }
        }

        if (key_info_f){
            if (    undefined == key_info_f.inc.send_delay
                ||  key_info_f.inc.send_delay >= 100)
            {
                key_info_f.inc.send_delay = 0;
                self.toggleFlyMode();
            }
        }

        if (key_info_c){
            if (    undefined == key_info_c.inc.send_delay
                ||  key_info_c.inc.send_delay >= 100)
            {
                key_info_c.inc.send_delay = 0;
                self.togglePointer();
            }
        }

        if (key_info_left){
            if (key_info_control){
            } else {
                if (    undefined == key_info_left.inc.send_delay
                    ||  key_info_left.inc.send_delay >= 100)
                {
                    key_info_left.inc.send_delay = 0;
                    self.rotateViewVector({
                        'params':   {
                            'yaw':  -5 * Math.PI/ 180
                        }
                    });
                }
            }
        } else if (key_info_right){
            if (key_info_control){
            } else {
                if (    undefined == key_info_right.inc.send_delay
                    ||  key_info_right.inc.send_delay >= 100)
                {
                    key_info_right.inc.send_delay = 0;
                    self.rotateViewVector({
                        'params':   {
                            'yaw':  5 * Math.PI / 180
                        }
                    });
                }
            }
        }

        if (key_info_a){
            if (    undefined == key_info_a.inc.send_delay
                ||  key_info_a.inc.send_delay >= 100)
            {
                key_info_a.inc.send_delay = 0;
                self.strafe(-1.000000000000001);
            }
        } else if (key_info_d){
            if (    undefined == key_info_d.inc.send_delay
                ||  key_info_d.inc.send_delay >= 100)
            {
                key_info_d.inc.send_delay = 0;
                self.strafe(1.000000000000001);
            }
        }

        if (key_info_up){
            if (    undefined == key_info_up.inc.send_delay
                ||  key_info_up.inc.send_delay >= 100)
            {
                key_info_up.inc.send_delay = 0;
                self.rotateViewVector({
                    'params':   {
                        'pitch': 5 * Math.PI/ 180 
                    }
                });
            }
        } else if (key_info_down){
            if (    undefined == key_info_down.inc.send_delay
                ||  key_info_down.inc.send_delay >= 100)
            {
                key_info_down.inc.send_delay = 0;
                self.rotateViewVector({
                    'params':   {
                        'pitch': -5 * Math.PI/ 180 
                    }
                });
            }
        }

        if (key_info_w){
            if (    undefined == key_info_w.inc.send_delay
                ||  key_info_w.inc.send_delay >= 100)
            {
                key_info_w.inc.send_delay = 0;
                self.accel({
                    accel:  1.000000000000001
                });
            }
        } else if (key_info_s){
            if (    undefined == key_info_s.inc.send_delay
                ||  key_info_s.inc.send_delay >= 100)
            {
                key_info_s.inc.send_delay = 0;
                self.accel({
                    accel:  -1.000000000000001
                });
            }
        }

        // TODO remove me
        //
        //if (a_cur_pressed_keys[KeyEvent.DOM_VK_RIGHT]){
        //    self.rotate(+1);
        //}
        //if (a_cur_pressed_keys[KeyEvent.DOM_VK_LEFT]){
        //    self.rotate(-1);
        //}
    },

    select: function(){
        var self        = this;
        var pos         = self.option('object_pos');
        var object_id   = self.option('object_id');

        self.m_is_selected = 0.1;

        getLogger().pdebug(1, "select object: '"+object_id+"'",
            {
                'pos':  pos,
            }
        );
    },

    deSelect: function(){
        var self        = this;
        var pos         = self.option('object_pos');
        var object_id   = self.option('object_id');

        self.m_is_selected = -1;

        getLogger().pdebug(1, "deselect object: '"+object_id+"'",
            {
                'pos':  pos,
            }
        );
    },
});

// ---------------- others ----------------

function getIPNoiseClassName(a_el)
{
    var css_classes     = [];
    var obj_class_name;
    var ipnoise_class_name;

    // get css classes
    css_classes = a_el
        .attr('class')
        .split(/\s+/);

    // search first Object* class name
    for (var i = 0;
        i < css_classes.length;
        i++)
    {
        var res;
        var cur_css_class;
        cur_css_class = css_classes[i];

        res = /(IPNoise.*)/.exec(
            cur_css_class
        );
        if (res){
            ipnoise_class_name = res[1];
            break;
        }
    }

    return ipnoise_class_name;
}

function getEvents()
{
    var packet = {
        method:     'getEvents',
        object_id:  getCurObjectId(),
        success:    function(data){
            var params = data.params    || {};
            var events = params.events  || {};
            for (var i = 0; i < events.length; i++){
                var event       = events[i];
                var object_id   = event.object_id;

                $("div[object_id='"+object_id+"']")
                    .each(function(){
                        var ipnoise_class_name;
                        var el = $(this);

                        ipnoise_class_name = getIPNoiseClassName(
                            el
                        );

                        if (undefined == el[ipnoise_class_name]){
                            getLogger().pfatal(
                                "el["+ipnoise_class_name+"]"
                                +" is undefined"
                            );
                        }

                        // ok, get jquery object and inform
                        // about events
                        el[ipnoise_class_name](
                            'apiEvent',
                            event
                        );
                    });
            }
            // wait new events
            getEvents();
        },
        failed:     function (data){
            // a little wait and try again (msec)
            setTimeout(getEvents, Math.random() * 5e3);
        }
    };

    var args = {
        'packet':     packet,
        'single':     1
    };
    sendApiPacket(args);
}

