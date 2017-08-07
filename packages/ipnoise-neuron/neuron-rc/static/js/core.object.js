// vim:syntax=javascript tabstop=4 expandtab

// ---------------- static ----------------

var g_modules_by_type = {};

function module_register(a_module_info)
{
    var type        = a_module_info.type;
    var module_info = undefined;

    if (    undefined   ==  type
        ||  null        === type)
    {
        getLogger().pfatal("missing argument: 'type'");
    }

    module_info = g_modules_by_type[type];
    if (module_info){
        getLogger().pfatal("module with type: '"+type+"'"
            +" already registered",
            module_info
        );
    }

    g_modules_by_type[type] = a_module_info;

    getLogger().pdebug(10, "register module: '"+type+"'");
}

function object_create(
    a_object_type,
    a_args)
{
    var module_info = undefined;
    var object      = undefined;
    var args        = {};

    module_info = g_modules_by_type[a_object_type];
    if (!module_info){
        getLogger().pfatal(
            "module with type: '"+a_object_type+"'"
            +" not found"
        );
    }

    getLogger().pdebug(0, 'call module_info.objectCreate',
        module_info
    );
    object = module_info.objectCreate();
    if (!object){
        getLogger().pfatal(
            "cannot create object"
            +" with type: '"+a_object_type+"'"
        );
    }

    if (a_args){
        args = a_args;
    }

    object.v_reset(args);

    if (!object.getLogger()){
        getLogger().pfatal("object.getLogger() failed\n",
            object
        );
    }

    return object;
}

// setup vertex buffer and attributes offsets
function vertex_array_bind(
    a_gl,
    a_vert_buff,
    a_shader)
{
    var self  = this;
    var gl    = a_gl;
    var attrs = a_shader.getAttrs();

    gl.bindBuffer(
        gl.ARRAY_BUFFER,
        a_vert_buff
    );

    if (attrs.aPosition > -1){
        gl.vertexAttribPointer(
            attrs.aPosition,
            3,
            gl.FLOAT,
            false,
            36,
            0                           // offset
        );
    }
    if (attrs.aNormal > -1){
        gl.vertexAttribPointer(
            attrs.aNormal,
            3,
            gl.FLOAT,
            false,
            36,
            12                          // offset
        );
    }
    if (attrs.aTexCoord > -1){
        gl.vertexAttribPointer(
            attrs.aTexCoord,
            2,
            gl.FLOAT,
            false,
            36,
            24                          // offset
        );
    }
    if (attrs.aFaceIndex > -1){
        gl.vertexAttribPointer(
            attrs.aFaceIndex,           // index
            1,                          // size
            gl.FLOAT,                   // type
            false,                      // normalized
            36,                         // stride
            32                          // offset
        );
    }
};

// ---------------- Object ----------------

function Object()
{
    var self = this;
}

Object.prototype.getObjectType = function()
{
    return 'core.object.object';
}

Object.prototype.v_reset = function(
    a_args)
{
    var self = this;

    console.log('super, args: '+a_args);

    self.m_object_logger   = a_args.logger || getLogger();
    self.m_viewer          = a_args.viewer;
    self.m_object_id       = 0;
    self.m_object_name     = '';

    self.worldVertexPositionBuffer       = undefined;
    self.worldVertexTextureCoordBuffer   = undefined;
    self.textures                        = [];
    self.ready                           = 0;

    self.m_pos          = vec3.create([0, 0, 0]);
    self.m_view_pos     = vec3.create([0, 0, -0.1]);
    self.m_view_up      = vec3.create([0, 0, 1]);
    self.m_view_strafe  = vec3.create([0, 0, 0]);
    self.m_view_ryp     = vec3.create([0, 0, 0]);
    self.m_view_zoom    = 0;

    self.m_rot          = quat4.create();
    self.m_tex          = vec3.create();
    self.m_mag          = 0;
    self.m_spot         = undefined;
    self.m_cube_buf     = {};
    self.m_scale        = 1;
    self.m_is_selected  = -1;
    self.m_size         = 1;
    self.m_selectedFace = 1;
    self.m_tweens       = [];
}

// --- id ---

Object.prototype.getObjectId = function()
{
    var self = this;
    return self.m_object_id;
}

Object.prototype.setObjectId = function(
    a_id)
{
    var self = this;
    self.m_object_id = a_id;
}

// --- name ---

Object.prototype.getObjectName = function()
{
    var self = this;
    return self.m_object_name;
}

Object.prototype.setObjectName = function(
    a_name)
{
    var self = this;
    self.m_object_name = a_name;
}

// --- window ---

Object.prototype.getWindow = function()
{
    var self = this;
    return self.m_object_window;
}

Object.prototype.setWindow = function(
    a_val)
{
    var self = this;
    self.m_object_window = a_val;
}

// --- document ---

Object.prototype.getDocument = function()
{
    var self = this;
    var win  = self.getWindow();
    var doc  = undefined;

    if (win){
        doc = win.document;
    }

    return doc;
}

// --- viewer ---

Object.prototype.getViewer = function()
{
    var self = this;
    return self.m_viewer;
}

Object.prototype.setViewer = function(
    a_viewer)
{
    var self = this;
    self.m_viewer = a_viewer;

    // self.initCubeBuffers();
    self.initAtomBuffers();
}

// --- logger ---

Object.prototype.getLogger = function()
{
    var self = this;
    return self.m_object_logger;
}

Object.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    //self.m_object_logger = a_logger;
}

// --- pos ---

Object.prototype.getPos = function()
{
    var self = this;
    return self.m_pos;
}

Object.prototype.setPos = function(
    a_pos)
{
    var self = this;

    if ('object' != typeof a_pos){
        self.getLogger().pfatal(
            "argument: 'a_pos' in not object"
            +" (must be object vec3)",
            a_pos
        );
    }

    self.m_pos = a_pos;
}

// --- view_pos ---

Object.prototype.getViewPos = function()
{
    var self = this;
    return self.m_view_pos;
}

Object.prototype.setViewPos = function(
    a_pos)
{
    var self = this;

    if ('object' != typeof a_pos){
        self.getLogger().pfatal(
            "argument: 'a_pos' in not object"
            +" (must be object vec3)",
            a_pos
        );
    }

    self.m_view_pos = a_pos;
}

// --- view_up ---

Object.prototype.getViewUp = function()
{
    var self = this;
    return self.m_view_up;
}

Object.prototype.setViewUp = function(
    a_up)
{
    var self = this;

    if ('object' != typeof a_up){
        self.getLogger().pfatal(
            "argument: 'a_pos' in not object"
            +" (must be object vec3)",
            a_up
        );
    }

    self.m_view_up = a_up;
}

// --- keys ---

Object.prototype.handleKeys = function(
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
}

// --- select ---

Object.prototype.select = function()
{
    var self        = this;
    var pos         = self.getObjectPos();
    var object_id   = self.getObjectId();

    self.m_is_selected = 0.1;

    self.getLogger().pdebug(1, "select object:"
        +" '"+object_id+"'",
        {
            'pos':  pos,
        }
    );
}

Object.prototype.deSelect = function()
{
    var self        = this;
    var pos         = self.getObjectPos();
    var object_id   = self.getObjectId();

    self.m_is_selected = -1;

    select.getLogger().pdebug(1, "deselect object:"
        +" '"+object_id+"'",
        {
            'pos':  pos,
        }
    );
}

// --- draw ---

Object.prototype.draw = function(
    a_pick_color)
{
    var self     = this;

    var viewer   = self.getViewer();
    var shaders  = viewer.getShaders();
    var gl       = viewer.getGl();
    var camera   = viewer.getCamera();
    var mvMatrix = camera.getMvMatrix();
    var pMatrix  = camera.getPMatrix();
    var zMatrix  = camera.getZMatrix();

    if (!mvMatrix){
        self.getLogger().pfatal("missing 'mvMatrix'");
    }

    if (!pMatrix){
        self.getLogger().pfatal("missing 'pMatrix'");
    }

    if (!zMatrix){
        self.getLogger().pfatal("missing 'zMatrix'");
    }

    if (!self.m_pos){
        self.getLogger().pfatal("missing 'm_pos'");
    }

    if (!self.m_view_pos){
        self.getLogger().pfatal("missing 'm_view_pos'");
    }

    if (undefined != a_pick_color){
        var shader_class = 'core.object.shader.atom';
        var shader       = shaders.use(shader_class);
        if (!shader){
            getLogger().pfatal("cannot init shader:"
                +" '"+shader_class+"'"
            );
        }

        var attrs       = shader.getAttrs();
        var uniforms    = shader.getUniforms();

        gl.uniformMatrix4fv(
            uniforms.uPMatrix,
            false,
            pMatrix
        );

        gl.uniformMatrix4fv(
            uniforms.uZMatrix,
            false,
            zMatrix
        );

        gl.bindBuffer(
            gl.ELEMENT_ARRAY_BUFFER,
            self.m_cube_buf.index
        );

        // setup vertex buffer and attributes offsets
        vertex_array_bind(
            gl,
            self.m_cube_buf.vert,
            shader
        );

        // rotate if need
        var localMatrix = mat4.fromRotationTranslation(
            self.m_rot,
            self.m_pos
        );
        mat4.scale(
            localMatrix,
            [
                self.m_scale,
                self.m_scale,
                self.m_scale
            ]
        );
        mat4.multiply(
            mvMatrix,
            localMatrix,
            localMatrix
        );

        gl.uniformMatrix4fv(
            uniforms.uMVMatrix,
            false,
            localMatrix
        );

        gl.uniform3f(
            uniforms.uPickColor,
            a_pick_color[0],
            a_pick_color[1],
            a_pick_color[2]
        );

        gl.drawElements(
            gl.POINTS,
            self.m_cube_buf.index.itemCount,
            gl.UNSIGNED_SHORT,
            0
        );
    } else {
        var shader_class = 'core.object.shader.atom';
        var shader       = shaders.use(shader_class);
        if (!shader){
            getLogger().pfatal("cannot init shader:"
                +" '"+shader_class+"'");
        }

        var attrs       = shader.getAttrs();
        var uniforms    = shader.getUniforms();

        // draw view vector
        if (0){
            var x1 = self.m_pos[X];
            var y1 = self.m_pos[Y];
            var z1 = self.m_pos[Z];

            var x2 = self.m_view_pos[X];
            var y2 = self.m_view_pos[Y];
            var z2 = self.m_view_pos[Z];

            var line_verts  = [
                // Pos,         normal,         tex uv,     face index
                x1, y1, z1,     0, 0, 0,        0, 0,       0,
                x2, y2, z2,     0, 0, 0,        0, 0,       0
            ];
            var index       = [ 0, 1 ];
            var line_buf    = {};

            line_buf.vert = gl.createBuffer();
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                line_buf.vert
            );
            gl.bufferData(
                gl.ARRAY_BUFFER,
                new Float32Array(line_verts),
                gl.STATIC_DRAW
            );

            line_buf.index = gl.createBuffer();
            gl.bindBuffer(
                gl.ELEMENT_ARRAY_BUFFER,
                line_buf.index
            );
            gl.bufferData(
                gl.ELEMENT_ARRAY_BUFFER,
                new Uint16Array(index),
                gl.STATIC_DRAW
            );
            line_buf.index.itemCount = index.length;

            // draw
            var shader_class = 'core.object.shader.atom';
            var shader       = shaders.use(shader_class);
            if (!shader){
                getLogger().pfatal("cannot init shader:"
                    +" '"+shader_class+"'"
                );
            }

            var attrs    = shader.getAttrs();
            var uniforms = shader.getUniforms();

            gl.uniformMatrix4fv(
                uniforms.uPMatrix,
                false,
                pMatrix
            );

            gl.uniformMatrix4fv(
                uniforms.uZMatrix,
                false,
                zMatrix
            );

            gl.bindBuffer(
                gl.ELEMENT_ARRAY_BUFFER,
                line_buf.index
            );

            // setup vertex buffer and attributes offsets
            vertex_array_bind(
                gl,
                line_buf.vert,
                shader
            );

            gl.uniform1f(
                uniforms.uScale,
                self.m_size
            );

            // rotate if need
            var localMatrix = mat4.fromRotationTranslation(
                self.m_rot,
                //vec3.create([ 0, 0, 0 ]) //pos
                self.m_pos
            );

            mat4.scale(
                localMatrix,
                [
                    self.m_scale,
                    self.m_scale,
                    self.m_scale
                ]
            );

            mat4.multiply(
                mvMatrix,
                localMatrix,
                localMatrix
            );

            gl.uniformMatrix4fv(
                uniforms.uMVMatrix,
                false,
                localMatrix
            );

            gl.uniform3f(
                uniforms.uOffset,
                self.m_tex[0],
                self.m_tex[1],
                self.m_tex[2]
            );

            // add information about
            // selection
            gl.uniform1f(
                uniforms.uSelected,
                self.m_is_selected
            );

            gl.uniform3f(
                uniforms.uPickColor,
                255,
                255,
                255
            );

            gl.drawElements(
                gl.LINES,
                line_buf.index.itemCount,
                gl.UNSIGNED_SHORT,
                0
            );
        }

        gl.uniformMatrix4fv(
            uniforms.uPMatrix,
            false,
            pMatrix
        );

        gl.uniformMatrix4fv(
            uniforms.uZMatrix,
            false,
            zMatrix
        );

        gl.bindBuffer(
            gl.ELEMENT_ARRAY_BUFFER,
            self.m_cube_buf.index
        );

        // setup vertex buffer and attributes offsets
        vertex_array_bind(
            gl,
            self.m_cube_buf.vert,
            shader
        );

        gl.uniform1f(
            uniforms.uScale,
            self.m_size
        );

        // rotate if need
        var localMatrix = mat4.fromRotationTranslation(
            self.m_rot,
            self.m_pos
        );

        mat4.scale(
            localMatrix,
            [
                self.m_scale,
                self.m_scale,
                self.m_scale
            ]
        );
        mat4.multiply(
            mvMatrix,
            localMatrix,
            localMatrix
        );

        gl.uniformMatrix4fv(
            uniforms.uMVMatrix,
            false,
            localMatrix
        );

        gl.uniform3f(
            uniforms.uOffset,
            self.m_tex[0],
            self.m_tex[1],
            self.m_tex[2]
        );

        // add information about
        // selection
        gl.uniform1f(
            uniforms.uSelected,
            self.m_is_selected
        );

        gl.uniform3f(
            uniforms.uPickColor,
            255,
            255,
            255
        );

        gl.drawElements(
            gl.POINTS,
            self.m_cube_buf.index.itemCount,
            gl.UNSIGNED_SHORT,
            0
        );
    }
}

Object.prototype.initAtomBuffers = function()
{
    // Set up buffers
    var self    = this;

    var viewer  = self.getViewer();
    var gl      = viewer.getGl();
    var verts   = [
        // Pos,         normal,         tex uv,     face index
        0, 0, 0,        -1, 0, 0,       0, 0,       0
    ];
    var index   = [ 0 ];

    self.m_cube_buf.vert = gl.createBuffer();
    gl.bindBuffer(
        gl.ARRAY_BUFFER,
        self.m_cube_buf.vert
    );
    gl.bufferData(
        gl.ARRAY_BUFFER,
        new Float32Array(verts),
        gl.STATIC_DRAW
    );

    self.m_cube_buf.index = gl.createBuffer();
    gl.bindBuffer(
        gl.ELEMENT_ARRAY_BUFFER,
        self.m_cube_buf.index
    );
    gl.bufferData(
        gl.ELEMENT_ARRAY_BUFFER,
        new Uint16Array(index),
        gl.STATIC_DRAW
    );
    self.m_cube_buf.index.itemCount = index.length;
}

Object.prototype.strafe = function(
    a_val)
{
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
}

Object.prototype.accel = function(
    a_args)
{
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

    var pos      = self.getPos();
    var view_pos = self.getViewPos();

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
}

Object.prototype.rotateViewVector = function(
    a_args)
{
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
}

Object.prototype.rotate_position = function(
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
}

// ---------------- module ----------------

var module_info = {
    'type':         Object.prototype.getObjectType(),
    'objectCreate': function(a_args){
        return new Object(a_args);
    }
};
module_register(module_info);

