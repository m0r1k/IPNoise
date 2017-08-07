// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseWebglObjectCamera",
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
            getLogger().pfatal("missing argument 'viewer'");
        }

        self.m_mvMatrix = mat4.create();
        self.m_pMatrix  = mat4.create();
        self.m_zMatrix  = mat4.create();

        mat4.identity(self.m_mvMatrix);
        mat4.identity(self.m_zMatrix);

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglObjectCamera');

        self.m_draw_objects_by_color    = {};
        self.m_draw_objects_by_id       = {};
        self.m_next_pick_color          = 100;
        self.m_selected                 = [];

        self.m_mouse_down               = false;
        self.m_first_mouse_X            = undefined;
        self.m_first_mouse_Y            = undefined;
        self.m_last_mouse_X             = undefined;
        self.m_last_mouse_Y             = undefined;
        self.m_fly_mode                 = 0;
        self.m_show_pick_fbuffer_flag   = 0;
        self.m_pick_fbuffer             = {};

        var gl = g_viewer.callWidgetMethod('getGl');
        if (!gl){
            self.getLogger().pfatal("cannot get GL viewer");
        }

        // create framebuffer for picking
        // http://www.songho.ca/opengl/gl_fbo.html
        self.m_pick_fbuffer.frame = gl.createFramebuffer();
        gl.bindFramebuffer(
            gl.FRAMEBUFFER,
            self.m_pick_fbuffer.frame
        );

        // create texture
        self.m_pick_fbuffer.color = gl.createTexture();
        gl.bindTexture(
            gl.TEXTURE_2D,
            self.m_pick_fbuffer.color
        );
        gl.texParameteri(
            gl.TEXTURE_2D,
            gl.TEXTURE_MAG_FILTER,
            gl.NEAREST
        );
        gl.texParameteri(
            gl.TEXTURE_2D,
            gl.TEXTURE_MIN_FILTER,
            gl.NEAREST
        );
        gl.texImage2D(
            gl.TEXTURE_2D,
            0,
            gl.RGBA,
            gl.viewportWidth,
            gl.viewportHeight,
            0,
            gl.RGBA,
            gl.UNSIGNED_BYTE,
            null
        );
        // create render buffer
        self.m_pick_fbuffer.depth = gl.createRenderbuffer();
        gl.bindRenderbuffer(
            gl.RENDERBUFFER,
            self.m_pick_fbuffer.depth
        );
        gl.renderbufferStorage(
            gl.RENDERBUFFER,
            gl.DEPTH_COMPONENT16,
            gl.viewportWidth,
            gl.viewportHeight
        );
        // attach texture to framebuffer
        gl.framebufferTexture2D(
            gl.FRAMEBUFFER,
            gl.COLOR_ATTACHMENT0,
            gl.TEXTURE_2D,
            self.m_pick_fbuffer.color,
            0
        );
        // attach render buffer to framebuffer
        gl.framebufferRenderbuffer(
            gl.FRAMEBUFFER,
            gl.DEPTH_ATTACHMENT,
            gl.RENDERBUFFER,
            self.m_pick_fbuffer.depth
        );

        // clean up
        gl.bindTexture(gl.TEXTURE_2D,        null);
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);
        gl.bindFramebuffer(gl.FRAMEBUFFER,   null);

        // OpenGL state stuff
        gl.viewport(
            0,
            0,
            gl.viewportWidth,
            gl.viewportHeight
        );
        gl.clearColor(0,0,0,1);
        gl.enable(gl.DEPTH_TEST);

        var aspect = gl.viewportWidth / gl.viewportHeight;

        mat4.perspective(
            25,                 // vertical field of view
            aspect,             // aspect ratio 
            ELECTRON_SIZE/10,   // near bounds of the frustum
            1000*ELECTRON_SIZE, // far bounds of the frustum
            self.m_pMatrix      // result output
        );

        // do not forget return self!
        return self;
    },

    // _setOption is called for each individual option
    // that is changing
    _setOption: function(
        a_key,
        a_val,
        a_args)
    {
        var self = this;
        var args = {
            'setup_attribute':  1,
            'handled':          0
        };
        copy_args(args, a_args);

        var update_mv_matrix = 0;

        if (    'object_pos'        == a_key
            ||  'object_view_pos'   == a_key
            ||  'object_view_up'    == a_key)
        {
            args.handled     = 1;
            update_mv_matrix = 1;
        }
 
        self._super(a_key, a_val, args);

        if (update_mv_matrix){
            self.updateMvMatrix();
        }
    },

    updateMvMatrix: function(){
        var self = this;

        var pos         = JSON_STR_TO_VEC3(self.option('object_pos'));
        var view_pos    = JSON_STR_TO_VEC3(self.option('object_view_pos'));
        var view_up     = JSON_STR_TO_VEC3(self.option('object_view_up'));
        var view_strafe = JSON_STR_TO_VEC3(self.option('object_view_strafe'));
        var view_ryp    = JSON_STR_TO_VEC3(self.option('object_view_ryp'));
        var view_zoom   = self.option('object_view_zoom') * 1;

        if (    undefined != pos
            &&  undefined != view_pos
            &&  undefined != view_up)
        {
            self.getLogger().pdebug(1, "camera updateMvMatrix",
                {
                    'pos':          pos,
                    'view_pos':     view_pos,
                    'view_up':      view_up,
                    'view_strafe':  view_strafe,
                    'view_ryp':     view_ryp,
                    'view_zoom':    view_zoom
                }
            );

            //var mv_matrix = self.getMvMatrix();
            mat4.lookAt(
                pos,
                view_pos,
                view_up,
                self.m_mvMatrix
            );
        }
    },

    getMvMatrix: function(){
        var self = this;
        return self.m_mvMatrix;
    },

    getPMatrix: function(){
        var self   = this;
        return self.m_pMatrix;
    },

    getZMatrix: function(){
        var self   = this;
        return self.m_zMatrix;
    },

    watchStart: function(){
        var self   = this;
        var params = {};
        var packet = {
            'method':   'watchStart',
            'params':   params,
            'success':  function(a_data){
                var params      = a_data.params;
                var draw_ids    = [];
                if (params.draw_ids){
                    draw_ids = params.draw_ids;
                }
                for (var i = 0; i < draw_ids.length; i++){
                    var draw_id = draw_ids[i];
                    createWidget({
                        'object_id':    draw_id,
                        'success':      function(a_data, a_widgets){
                            var camera = g_viewer.callWidgetMethod(
                                'getCurCamera'
                            );
                            for (var i = 0; i < a_widgets.length; i++){
                                var widget = a_widgets[i];
                                camera.callWidgetMethod('addForDraw', widget);
                            }
                        },
                        'args': {
                            'viewer': self.getViewer()
                        }
                    });
                }
            }
        };
        var data = {
            'packet':   packet
        };
        self.sendApiPacket(data);
    },

    getSelected: function(){
        var self = this;
        return self.m_selected;
    },

    getShowPickFbufferFlag: function(
        a_arg)
    {
        var self = this;
        if (undefined != a_arg){
            self.m_show_pick_fbuffer_flag = a_arg;
        }
        return self.m_show_pick_fbuffer_flag;
    },

    setFlyMode: function(
        a_arg)
    {
        var self = this;
        self.m_fly_mode = a_arg;

        var msg = 'Camera in ';
        msg += a_arg ? 'fly' : 'walk';
        msg += ' mode';
        self.getLogger().pinfo(msg);
    },

    getFlyMode: function(){
        var self = this;
        return self.m_fly_mode;
    },

    toggleFlyMode: function(){
        var self = this;
        var cur_val = self.getFlyMode();
        self.setFlyMode(!cur_val);
    },

    getObjectByColor: function(a_color){
        var self = this;
        var ret;
        ret = self.m_draw_objects_by_color[a_color];
        //getLogger().pdebug(1, "getObjectByColor: '"+a_color+"',"
        //    +" ret: '"+ret+"'"
        //    +" m_draw_objects_by_color: ",
        //    self.m_draw_objects_by_color
        //);

        return ret;
    },

    createCurObject: function(){
        var self = this;

        if (!self.element){
            getLogger().pfatal(
                "cannot create object without viewer"
            );
        }

        createWidget({
            'object_id':    getCurObjectId(),
            'args':         {
                'viewer': self.element,
            },
            'success':      function(a_data, a_widgets){
                for (var i = 0; i < a_widgets.length; i++){
                    var widget = a_widgets[i];
                    self.addForDraw(widget);
                }
            }
        });
    },
    
    getNextColor: function(){
        var self = this;
        var color = self.m_next_pick_color;
        self.m_next_pick_color += 100;
        return color;
    },

    addForDraw: function(
        a_obj)
    {
        var self  = this;
        var color = self.getNextColor();
        var id    = a_obj.callWidgetMethod('option', 'object_id');
        var type  = a_obj.callWidgetMethod('option', 'object_type');

        if (undefined == id){
            self.getLogger().pfatal(
                "cannot get object id",
                a_obj
            );
        }

        if (undefined == type){
            self.getLogger().pfatal(
                "cannot get object type",
                a_obj
            );
        }

        self.getLogger().pdebug(1, "add for draw object ID: '"+id+"'"
            +" type: '"+type+"'"
        );

        if (!self.m_draw_objects_by_id[id]){
            self.m_draw_objects_by_color[color] = a_obj;
            self.m_draw_objects_by_id[id]       = a_obj;
        }
    },

    setMouseDown: function(a_arg){
        var self = this;
        self.m_mouse_down = a_arg;
    },

    isMouseDown: function(){
        var self = this;
        var ret  = 0;
        if (self.m_mouse_down){
            ret = 1;
        }
        return ret;
    },

    // first
    getFirstMouseX: function(){
        var self = this;
        return self.m_first_mouse_X;
    },

    setFirstMouseX: function(a_arg){
        var self = this;
        self.m_first_mouse_X = a_arg;
    },

    getFirstMouseY: function(){
        var self = this;
        return self.m_first_mouse_Y;
    },

    setFirstMouseY: function(a_arg){
        var self = this;
        self.m_first_mouse_Y = a_arg;
    },

    // last
    getLastMouseX: function(){
        var self = this;
        return self.m_last_mouse_X;
    },

    setLastMouseX: function(a_arg){
        var self = this;
        self.m_last_mouse_X = a_arg;
    },

    getLastMouseY: function(){
        var self = this;
        return self.m_last_mouse_Y;
    },

    setLastMouseY: function(a_arg){
        var self = this;
        self.m_last_mouse_Y = a_arg;
    },

    handleMouseDown: function(a_ev){
        var self   = this;
        var canvas = self.getViewer().callWidgetMethod('getCanvas');

        var first_mouse_X;
        var first_mouse_Y;

        var last_mouse_X;
        var last_mouse_Y;

        self.setMouseDown(true);

        pos_x = a_ev.offsetX
            ?   a_ev.offsetX
            :   (a_ev.pageX - canvas.get(0).offsetLeft);

        pos_y = a_ev.offsetY
            ?   a_ev.offsetY
            :   (a_ev.pageY - canvas.get(0).offsetTop);

        first_mouse_X = last_mouse_X = pos_x;
        first_mouse_Y = last_mouse_Y = pos_y;

        self.getLogger().pdebug(10, "handleMouseDown:\n"
            +"  first_mouse_X: '"+first_mouse_X+"'\n"
            +"  first_mouse_Y: '"+first_mouse_Y+"'\n"
        );

        // first
        self.setFirstMouseX(first_mouse_X);
        self.setFirstMouseY(first_mouse_Y);

        // last
        self.setLastMouseX(last_mouse_X);
        self.setLastMouseY(last_mouse_Y);

        getLogger().pdebug(5, "handleMouseDown", {
            'pos_x':    pos_x,
            'pos_y':    pos_y
        });
    },

    getPickFbuffer: function(){
        var self = this;
        return self.m_pick_fbuffer;
    },

    handleMouseUp: function(
        a_ev)
    {
        var self    = this;;
        var gl      = g_viewer.callWidgetMethod('getGl');
        var canvas  = g_viewer.callWidgetMethod('getCanvas');

        var first_mouse_X   = self.getFirstMouseX();
        var first_mouse_Y   = self.getFirstMouseY();
        var pick_fbuffer    = self.getPickFbuffer();

        self.setMouseDown(false);

        pos_x = a_ev.offsetX
            ?   a_ev.offsetX
            :   (a_ev.pageX - canvas.get(0).offsetLeft);

        pos_y = a_ev.offsetY
            ?   a_ev.offsetY
            :   (a_ev.pageY - canvas.get(0).offsetTop);

        self.getLogger().pdebug(10, "handleMouseUp:\n"
            +"  first_mouse_X: '"+first_mouse_X+"'\n"
            +"  first_mouse_Y: '"+first_mouse_Y+"'\n"
        );

        if (    Math.abs(first_mouse_X - pos_x) < 5
            &&  Math.abs(first_mouse_Y - pos_y) < 5)
        {
            var pickColor = new Uint8Array(4);
            gl.bindFramebuffer(
                gl.FRAMEBUFFER,
                pick_fbuffer.frame
            );

            // clear screen
            gl.clear(
                gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT
            );

            self.drawScene(1);

            gl.readPixels(
                pos_x,
                gl.viewportHeight - pos_y,
                1,
                1,
                gl.RGBA,
                gl.UNSIGNED_BYTE,
                pickColor
            );
            gl.bindFramebuffer(
                gl.FRAMEBUFFER,
                null
            )
            var color = pickColor[0]
                + (pickColor[1] << 8)
                + (pickColor[2] << 16);

            var cur_id      = self.option('object_id');
            var object      = self.getObjectByColor(color);
            var object_id   = undefined;

            if (object){
                object_id = object.callWidgetMethod(
                    'option',
                    'object_id'
                );
            }

            if (    object_id
                &&  cur_id != object_id)
            {
                self.deSelect();
                self.move({
                    'params': {
                        'object_id':    object_id
                    }
                });
            } else {
                self.select();
            }
        }
    },

    isActiveCamera: function(){
        var self    = this;
        var cam     = undefined;
        var cam_id  = undefined;
        var id      = self.option('object_id');
        var ret     = 0;

        cam = g_viewer.callWidgetMethod('getCamera');
        if (cam){
            cam_id = cam.callWidgetMethod('option', 'object_id');
        }
        if (cam_id == id){
            ret = 1;
        }

        return ret;
    },

    drawScene: function(a_picking){
        var self = this;

        var show_pick_buff_flag = self.getShowPickFbufferFlag();
        for (var color in self.m_draw_objects_by_color){
            var color_vec = undefined;
            var widget    = self.m_draw_objects_by_color[color];

            widget.callWidgetMethod('animate');

            if (    a_picking
                ||  show_pick_buff_flag)
            {
                color_vec = vec3.create([
                    (color & 0xff),
                    (color & 0xff00)   >> 8,
                    (color & 0xff0000) >> 16,
                ]);
            }

            widget.callWidgetMethod('draw', color_vec);
        }
    },

    // ---------------- api ----------------

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        // maybe super event?
        self._super(a_event);
    },

    animate: function(){},

    draw: function(a_picking){
        var self = this;
        self._super(a_picking);
    },

    select: function(){},

    deSelect: function(){},

    handleKeys: function(
        a_cur_pressed_keys)
    {
        var self = this;
        self._super(a_cur_pressed_keys);
    },

    handleMouseWheel: function(a_ev){
        var self     = this;
        /*
        var move     = a_ev.wheelDelta/240;
        var z_matrix = self.getZMatrix();

        if (    move < 0
            ||  z_matrix[14] > -2)
        {
            mat4.translate(
                z_matrix,
                [0, 0, a_ev.wheelDelta/240]
            );
        }
        */
    },

    // -------------------------------------
});
