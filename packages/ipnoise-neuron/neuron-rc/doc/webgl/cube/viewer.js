function Viewer()
{
    var self = this;

    self.m_name                 = 'viewer';
    self.m_shaders              = null;
    self.m_gl                   = null;
    self.m_camera               = null;
    self.m_mvMatrix             = mat4.create();
    self.m_pMatrix              = mat4.create();
    self.m_pick_fbuffer         = {};
    self.m_fps                  = 25;
    self.m_pointer_locked       = 0;
    self.m_webgl_inited         = 0;
    self.m_neurons_by_color     = {};
    self.m_next_pick_color      = 100;
    self.m_selected             = [];

    self.m_tick_time_ms_cur     = 0;
    self.m_tick_time_ms_prev    = 0;
    self.m_tick_time_ms_elapsed = 0;
    self.m_currentlyPressedKeys = {};

    self.m_logger = new Logger();
    self.m_logger.setPrefix('viewer');

    $(document).on("webkitfullscreenchange"
        +" mozfullscreenchange"
        +" fullscreenchange",
        function(){
            var canvas = $('#webGLCanvas').get(0);
            if (self.isFullScreen()){
                canvas.old_width    = canvas.width;
                canvas.old_height   = canvas.height;
                canvas.width        = window.innerWidth;
                canvas.height       = window.innerHeight;

                if (!self.m_webgl_inited){
                    self.init_webgl();
                }
                $('#screen').show();
            } else {
                canvas.width        = canvas.old_width;
                canvas.height       = canvas.old_height;

                $('#screen').hide();
            }
        }
    );

    $(document).on("webkitpointerlockchange"
        +" mozpointerlockchange"
        +" pointerlockchange",
        function(a_ev){
            var el  = $('#webGLCanvas').get(0);
            var doc = document;
            if (    doc.pointerLockElement       === el
                ||  doc.mozPointerLockElement    === el
                ||  doc.webkitPointerLockElement === el)
            {
                self.m_pointer_locked = 1;
            } else {
                self.m_pointer_locked = 0;
            }
        }
    );

    self.m_mouse_down   = false;

    self.m_first_mouse_X = null;
    self.m_first_mouse_Y = null;
    self.m_last_mouse_X  = null;
    self.m_last_mouse_Y  = null;

    var pitch       = 0;
    var pitchRate   = 0;

    var yaw         = 0;
    var yawRate     = 0;

    var roll        = 0;
    var rollRate    = 0;

    var xPos        = 0;
    var yPos        = 0.4;
    var zPos        = 0;

    var speed       = 0;

    var g_tweens    = [];

    function handleKeys(){
        var self = this;
        if (self.m_selected.length){
            for (var i = 0; i < self.m_selected.length; i++){
                var obj = self.m_selected[i];
                obj.handleKeys();
            }
        } else {
            self.m_camera.handleKeys();
        }
    }

    var tickTimeMs                  = 0;
    var tickLastTimeMs              = 0;
    var tickElapsedTimeMs           = 0;
    var tickElapsedRotateTimeMs     = 0;
    var tickElapsedPointerTimeMs    = 0;

    function handleMouseWheel(event){
        var move = event.wheelDelta/240;

        if (    move < 0
            ||  self.m_pMatrix[14] > -2)
        {
            mat4.translate(
                self.m_pMatrix,
                [0, 0, event.wheelDelta/240]
            );
        }

        // Don't scroll the page as well
        return false;
    }

    function handleScrambleButton(scramble){
        cube.scramble(scramble);
    }

    function handleGapRadio(size){
        cube.setGap(size);
    }
}

Viewer.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    self.m_logger = a_logger;
}

Viewer.prototype.getLogger = function()
{
    var self = this;
    return self.m_logger;
}

Viewer.prototype.getName = function()
{
    var self = this;
    return self.m_name;
}

Viewer.prototype.getGl = function()
{
    var self = this;
    var gl   = self.m_viewer.getGl();
    return gl;
}

Viewer.prototype.pointerLock = function(
    a_el)
{
    var self = this;

    self.getLogger().pinfo('pointerLock');
    var el = a_el;
    if (!el){
        el = $('#webGLCanvas').get(0);
    }

    // search function
    el.requestPointerLock
        = el.requestPointerLock
            ||  el.mozRequestPointerLock
            ||  el.webkitRequestPointerLock;

    // attempt to lock
    el.requestPointerLock();
}

Viewer.prototype.pointerUnLock = function(
    a_el)
{
    var self = this;

    self.getLogger().pinfo('pointerUnLock');
    var el = a_el;
    if (!el){
        el = document;
    }
    el.exitPointerLock
        = el.exitPointerLock
            ||  el.mozExitPointerLock
            ||  el.webkitExitPointerLock;

    // attempt to unlock
    el.exitPointerLock();
}

Viewer.prototype.togglePointer = function()
{
    var self = this;

    var state = self.m_pointer_locked;
    if (state){
        self.pointerUnLock();
    } else {
        self.pointerLock();
    }
}

Viewer.prototype.isFullScreen = function(
    a_el)
{
    var self = this;

    var el = a_el;
    if (!el){
        el = document;
    }
    var ret = el.fullscreenEnabled
        || el.mozFullscreenEnabled
        || el.webkitIsFullScreen;

    return ret;
}

Viewer.prototype.enterFullscreen = function()
{
    var self = this;
    var el   = $('#webGLCanvas').get(0);

    if (el.webkitRequestFullScreen){
        el.webkitRequestFullScreen(
            Element.ALLOW_KEYBOARD_INPUT
        );
    } else {
        el.mozRequestFullScreen();
    }
}

Viewer.prototype.init_webgl = function()
{
    var self = this;

    var canvas = $('#webGLCanvas').get(0);

    var gl = WebGLUtils.setupWebGL(canvas);
    self.m_gl = gl;

    gl.viewportWidth    = canvas.width;
    gl.viewportHeight   = canvas.height;

    canvas.onmousedown  = self.handleMouseDown;
    document.onmouseup  = self.handleMouseUp;
    canvas.onmousemove  = self.handleMouseMove;
    canvas.onmousewheel = self.handleMouseWheel;

    document.onkeyup    = self.handleKeyUp;
    document.onkeydown  = self.handleKeyDown;

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

    mat4.perspective(
        25,
        gl.viewportWidth / gl.viewportHeight,
        0.1,
        100.0,
        self.m_pMatrix
    );
    mat4.identity(self.m_mvMatrix);

    self.m_shaders = new Shaders(self);

    // add camera
    self.m_camera = new Camera(self);
    self.m_camera.position_camera(
        0, 0, -20,
        0, 0, 0,
        0, 1, 0
    );
    self.addForDraw(self.m_camera);

    var cube;

    // add cube
    cube = new Cube(
        self,
        3,
        vec3.create(
            [-0.8, 0, 0]
        )
    );
    self.addForDraw(cube);

    // add cube
    cube = new Cube(
        self,
        3,
        vec3.create(
            [0.8, 0, 0]
        )
    );
    self.addForDraw(cube);

    // mark as inited
    self.m_webgl_inited = 1;

    // start loop
    self.loop();
}

Viewer.prototype.getNextColor = function()
{
    var self = this;
    var color = self.m_next_pick_color;
    self.m_next_pick_color += 100;
    return color;
}

Viewer.prototype.addForDraw = function(
    a_obj)
{
    var self  = this;
    var color = self.getNextColor();

    // TODO XXX check object duplicates here
    // 20140729 morik@

    self.m_neurons_by_color[color] = a_obj;
}

Viewer.prototype._draw = function(
    a_picking)
{
    var self = this;

    for (var color in self.m_neurons_by_color){
        var obj = self.m_neurons_by_color[color];
        obj.animate();
        var color_vec = undefined;

        if (    a_picking
            ||  self.m_camera.showPickBuffer())
        {
            color_vec = vec3.create([
                (color & 0xff),
                (color & 0xff00)   >> 8,
                (color & 0xff0000) >> 16,
            ]);
        }

        obj.draw(color_vec);
    }
}

Viewer.prototype.tick = function()
{
    var self = this;
    var gl   = self.getGl();

    // calculate timings
    self.m_tick_time_ms_cur = new Date().getTime();
    if (self.m_tick_time_ms_prev){
        self.m_tick_time_ms_elapsed = self.m_tick_time_ms_cur
            - self.m_tick_time_ms_prev;
    }

    // select default frame buffer
    gl.bindFramebuffer(
        gl.FRAMEBUFFER,
        null
    );

    // clear screen
    gl.clear(
        gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT
    );

    // calculate timings
    tickTimeMs = new Date().getTime();
    if (tickLastTimeMs){
        tickElapsedTimeMs = tickTimeMs - tickLastTimeMs;
    }
    tickElapsedRotateTimeMs  += tickElapsedTimeMs;
    tickElapsedPointerTimeMs += tickElapsedTimeMs;

    handleKeys();

    _draw();

    // store last tick time
    self.m_tick_time_ms_prev = self.m_tick_time_ms_cur;
}

Viewer.prototype.loop = function()
{
    var self = this;

    setTimeout(function(){
        self.tick();
        requestAnimationFrame(self.loop);
        // Drawing code goes here
    }, 1000 / self.m_fps);
}

Viewer.prototype.handleMouseDown = function(
    a_ev)
{
    var self = this;

    self.m_mouse_down = true;

    var canvas = $('#webGLCanvas').get(0);

    pos_x = a_ev.offsetX
        ?   a_ev.offsetX
        :   (a_ev.pageX - canvas.offsetLeft);

    pos_y = a_evt.offsetY
        ?   a_ev.offsetY
        :   (a_ev.pageY - canvas.offsetTop);

    self.m_first_mouse_X = m_last_mouse_X = pos_x;
    self.m_first_mouse_Y = m_last_mouse_Y = pos_y;
}

Viewer.prototype.handleMouseUp = function(
    a_ev)
{
    var self = this;
    var gl   = self.getGl();

    self.m_mouse_down = false;

    var canvas = $('#webGLCanvas').get(0);

    pos_x = a_ev.offsetX
        ?   a_ev.offsetX
        :   (a_ev.pageX - canvas.offsetLeft);

    pos_y = a_ev.offsetY
        ?   a_ev.offsetY
        :   (a_ev.pageY - canvas.offsetTop);

    if (    Math.abs(self.m_first_mouse_X - pos_x) < 5
        &&  Math.abs(self.m_first_mouse_Y - pos_y) < 5)
    {
        var pickColor = new Uint8Array(4);
        gl.bindFramebuffer(
            gl.FRAMEBUFFER,
            self.m_pick_fbuffer.frame
        );

        // clear screen
        gl.clear(
            gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT
        );

        self._draw(1);

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

        var obj = self.m_neurons_by_color[color];
        self.getLogger().pdebug(10,
            "handleMouseUp, select object:", {
            'obj':      obj,
            'color':    color
        });
        if (obj){
            self.select(obj);
        } else {
            self.deSelectAll();
        }
    }
}

Viewer.prototype.select = function(
    a_obj)
{
    var self = this;
    // TODO XXX check duplicate here
    // 20140729 morik@
    self.m_selected.push(a_obj);
    a_obj.select();
}

Viewer.prototype.deSelectAll = function()
{
    var self = this;
    while (self.m_selected.length){
        var obj = self.m_selected.shift();
        self.deSelect(obj);
    }
}

Viewer.prototype.deSelect = function(
    a_obj)
{
    var self = this;
    // TODO XXX delete from array!
    // 20140729 morik@
    a_obj.deSelect();
}

Viewer.prototype.parseMouseMoveEvent = function(
    a_ev)
{
    var self = this;

    var ret = {
        move:   {
            x:  0,
            y:  0
        }
    };

    ret.move.x = a_ev.movementX
        || a_ev.mozMovementX
        || a_ev.webkitMovementX
        || 0;

    ret.move.y = a_ev.movementY
        || a_ev.mozMovementY
        || a_ev.webkitMovementY
        || 0;

    return ret;
}

Viewer.prototype.handleMouseMoveCamera = function(
    a_ev)
{
    var self = this;

    var ev = self.parseMouseMoveEvent(a_ev);
    yaw     -= ev.move.x/2;
    pitch   -= ev.move.y/2;
}

Viewer.prototype.handleMouseMoveScene = function(
    a_ev)
{
    var self = this;

    var canvas = $('#webGLCanvas').get(0);

    if (!self.m_mouse_down){
        return;
    }

    pos_x = a_ev.offsetX
        ?   (a_ev.offsetX)
        :   a_ev.pageX - canvas.offsetLeft;

    pos_y = a_ev.offsetY
        ?   (a_ev.offsetY)
        :   a_ev.pageY - canvas.offsetTop;

    self.m_last_mouse_X = pos_x;
    self.m_last_mouse_Y = pos_y;
}

Viewer.prototype.handleMouseMove = function(
    a_ev)
{
    var self = this;

    if (self.m_pointer_locked){
        self.handleMouseMoveCamera(a_ev);
    } else {
        self.handleMouseMoveScene(a_ev);
    }
}

Viewer.prototype.handleKeyDown = function(
    a_ev)
{
    var self = this;
    var code = a_ev.keyCode;

    self.getLogger().pdebug(10,
        "key down, code: '"+code+"'"
    );

    self.m_currentlyPressedKeys[code] = true;
}

Viewer.prototype.handleKeyUp = function(
    a_ev)
{
    var self = this;
    var code = a_ev.keyCode;

    self.getLogger().pdebug(10,
        "key up, code: '"+code+"'"
    );

    self.m_currentlyPressedKeys[code] = false;
}

Viewer.prototype.getGl = function()
{
    var self = this;
    return self.m_gl;
}

