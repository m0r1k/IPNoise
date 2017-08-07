// ---------------- static ----------------

function timer_draw(
    a_viewer)
{
    var fps = a_viewer.getFps();

    //getLogger().pinfo("max fps: '"+fps+"'\n");

    a_viewer.tick();
    //requestAnimationFrame(timer_draw);

    setTimeout(
        function (){
            timer_draw(a_viewer)
        },
        1000 / fps
    );
}

var color = 1;
var fps   = 1000000;

function move(
    a_viewer)
{
    var camera = a_viewer.getCamera();
    var object  = camera.m_draw_objects_by_color[color++];
    if (!object){
        color   = 1;
        object  = camera.m_draw_objects_by_color[color++];
    }
    var pos     = object.getPos();
    var new_pos = vec3.create([
        pos[X],
        pos[Y],
        pos[Z] + ELECTRON_SIZE/100,
    ]);
    object.setPos(new_pos);

    //setTimeout(
    //    function (){
    //        move(a_viewer)
    //    },
    //    1000 / fps
    //);
}

function moveAll(
    a_viewer)
{
    var camera = a_viewer.getCamera();

    do {
        var object = camera.m_draw_objects_by_color[color++];
        if (!object){
            color = 1;
            break;
        }
        var pos     = object.getPos();
        var new_pos = vec3.create([
            pos[X],
            pos[Y],
            pos[Z] + ELECTRON_SIZE / 70,
        ]);
        object.setPos(new_pos);
    } while (1);
}

// ---------------- Viewer ----------------

function ObjectViewer()
{
    var self = this;
}

ObjectViewer.prototype                  = new Object;
ObjectViewer.prototype.getObjectType    = function()
{
    return 'core.object.viewer';
}

ObjectViewer.prototype.v_reset = function(
    a_args)
{
    var self    = this;
    var args    = {
        'window': undefined
    };
    copy_args(args, a_args);

    if (!args.window){
        self.getLogger().pfatal(
            "missing argument: 'window'"
        );
    }

    // init super
    Object.prototype.v_reset.apply(self, [ args ]);

    // init us
    var win = args.window;

    // get window info
    //var win       = $(window);
    var win_height  = win.innerHeight; // win.height();
    var win_width   = win.innerWidth;  // win.width();

    // prepare elements
    self.m_inner = $('<div>');
    self.m_inner.attr('class', 'inner');
    self.m_inner.attr('style', 'text-align: center;');

    self.m_canvas = $('<canvas>');
    self.m_canvas.hide();
    self.m_canvas.attr('id',       'webGLCanvas');
    self.m_canvas.attr('width',    '800');
    self.m_canvas.attr('height',   '600');
    self.m_inner.append(self.m_canvas);

    // create start button
    self.m_start_button = $('<button>');
    self.m_start_button.css('position',  'absolute');
    self.m_start_button.css('top',  win_height/2 - 50 + 'px');
    self.m_start_button.css('left', win_width/2  - 50 + 'px');
    self.m_start_button.html('Start');
    self.m_start_button.on('click',  self.enterFullscreen);
    self.m_inner.append(self.m_start_button);

    self.m_shaders                  = undefined;
    self.m_gl                       = undefined;
    self.m_cameras                  = [];
    self.m_fps                      = 25;
    self.m_mouse_down               = 0;
    self.m_last_mouse_X             = undefined;
    self.m_last_mouse_Y             = undefined;
    self.m_pointer_locked           = 0;
    self.m_webgl_inited             = 0;

    self.m_tick_time_ms_cur         = 0;
    self.m_tick_time_ms_prev        = 0;
    self.m_tick_time_ms_elapsed     = 0;

    $(document).on("webkitfullscreenchange"
        +" mozfullscreenchange"
        +" fullscreenchange",
        function (a_ev){
            self.fullScreenChanged(a_ev)
        }
    );

    $(document).on("webkitpointerlockchange"
        +" mozpointerlockchange"
        +" pointerlockchange",
        function (a_ev){
            self.pointerLockChanged(a_ev);
        }
    );

}

ObjectViewer.prototype.appendTo = function(
    a_el)
{
    var self = this;

    a_el.append(self.m_inner);
}

/*
ObjectViewer.prototype.handleKeys = function()
{
    var self      = this;
    var m_camera  = self.getCamera();

    if (m_camera){
        m_camera.handleKeys(self.m_cur_pressed_keys);
    }
}
*/

ObjectViewer.prototype.handleMouseWheel = function(
    a_ev)
{
    var self    = this;
    var camera  = self.getCamera();
    if (camera){
        camera.handleMouseWheel(a_ev);
    }
    // Don't scroll the page as well
    return false;
}

ObjectViewer.prototype.handleScrambleButton = function(
    a_scramble)
{
    // randomize/solve puzzle
    cube.scramble(a_scramble);
}

ObjectViewer.prototype.handleGapRadio = function(
    a_size)
{
    // gap size 0 - none, 1 - small, 2 - big
    cube.setGap(a_size);
}

ObjectViewer.prototype.pointerLock = function(
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

ObjectViewer.prototype.pointerUnLock = function(
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

ObjectViewer.prototype.pointerToggle = function()
{
    var self = this;

    var state = self.m_pointer_locked;
    if (state){
        self.pointerUnLock();
    } else {
        self.pointerLock();
    }
}

ObjectViewer.prototype.pointerLockChanged = function(
    a_ev)
{
    var self    = this;
    var canvas  = self.m_canvas;
    var doc     = document;

    if (    doc.pointerLockElement       === el
        ||  doc.mozPointerLockElement    === el
        ||  doc.webkitPointerLockElement === el)
    {
        self.m_pointer_locked = 1;
    } else {
        self.m_pointer_locked = 0;
    }
}

ObjectViewer.prototype.isFullScreen = function(
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

ObjectViewer.prototype.enterFullscreen = function(
    a_el)
{
    var self    = this;
    var target  = $(a_el.target);
    var canvas  = target.parent().find('canvas');
    var el      = canvas.get(0);

    if (el.webkitRequestFullScreen){
        el.webkitRequestFullScreen(
            Element.ALLOW_KEYBOARD_INPUT
        );
    } else {
        el.mozRequestFullScreen();
    }
}

ObjectViewer.prototype.init_webgl = function()
{
    var self = this;

    if (self.m_webgl_inited){
        return;
    }

    var doc     = $(document);
    var canvas  = self.m_canvas.get(0);

    self.m_gl                = WebGLUtils.setupWebGL(canvas);
    self.m_gl.viewportWidth  = canvas.width;
    self.m_gl.viewportHeight = canvas.height;

    // mouse
    self.m_canvas.mousedown(
        function (a_ev){
            self.handleMouseDown(a_ev);
        }
    );
    doc.mouseup(
        function (a_ev){
            self.handleMouseUp(a_ev)
        }
    );
    self.m_canvas.mousemove(
        function (a_ev){
            self.handleMouseMove(a_ev);
        }
    );
    self.m_canvas.onmousewheel = function(a_ev){
        var target = $(a_ev.target);
        // TODO use target here
        g_viewer.handleMouseWheel(a_ev);
    }

    // init shaders
    self.m_shaders = new ObjectShaders();
    self.m_shaders.setViewer(self);

    // init camera
    self.getCreateCamera();

    // init objects
    //self.initObjects();

    // mark as inited
    self.m_webgl_inited = 1;

    // start draw
    timer_draw(self);

    // start move
    //move(self);
}

ObjectViewer.prototype.fullScreenChanged = function(
    a_ev)
{
    var self          = this;
    var is_fullscreen = self.isFullScreen();
    var canvas        = self.getCanvas();
    var win           = self.getWindow();

    if (is_fullscreen){
        canvas.attr(
            'style',
            'text-align: center;'
        );

        canvas.show();
        canvas.old_width     = canvas.width;
        canvas.old_height    = canvas.height;
        canvas.get(0).width  = win.innerWidth;
        canvas.get(0).height = win.innerHeight;

        self.init_webgl();
    } else {
        self.m_start_button.html('Full screen');

        canvas.attr(
            'style',
            'text-align: center;'
            +'opacity: 0.5;'
            +'z-index: -1;'
        );

        //canvas.hide();
        canvas.get(0).width  = window.innerWidth  * 0.9;
        canvas.get(0).height = window.innerHeight * 0.9;
    }
}

ObjectViewer.prototype.initObjects = function()
{
    var self    = this;
    var camera  = self.getCamera();

    var x, y, z;
    var count   = 0;
    var step    = ELECTRON_SIZE / 5;
    var min     = -7 * step;
    var max     = +7 * step;

    for (x = min; x < max; x += step){
        for (y = min;y < max; y += step){
            for (z = min; z < max; z += step){

                var object = new Object();
                object.setObjectId(x+':'+y+':'+z);
                object.setViewer(self);

                object.setPos(vec3.create([x, y, z]));
                object.setViewPos(
                    vec3.create([0, 0, ELECTRON_SIZE])
                );
                camera.addForDraw(object);

                count++;
            }
        }
    }

    console.log('count: '+count);
}

ObjectViewer.prototype.getCamera = function(
    a_name)
{
    var self    = this;
    var camera  = undefined;
    var name    = 'default';

    if (    a_name
        &&  a_name.length)
    {
        name = a_name;
    }

    camera = self.m_cameras[name];

    return camera;
}

ObjectViewer.prototype.getCreateCamera = function(
    a_name)
{
    var self    = this;
    var camera  = undefined;
    var name    = 'default';

    if (    a_name
        &&  a_name.length)
    {
        name = a_name;
    }

    camera = self.getCamera(name);
    if (!camera){
        camera = new ObjectCamera();
        camera.setObjectName(name);
        camera.setViewer(self);
        camera.setPos(vec3.create([
            10*ELECTRON_SIZE,
            20*ELECTRON_SIZE,
            -ELECTRON_SIZE
        ]));
        camera.setViewPos(vec3.create([0, 0, 0]));
        camera.init_webgl();
        self.m_cameras[name] = camera;
    }
}

ObjectViewer.prototype.selectCamera = function(
    a_name)
{
    var self    = this;
    var camera  = undefined;
    var name    = 'default';

    if (    a_name
        &&  a_name.length)
    {
        name = a_name;
    }

    camera = self.getCamera(name);
    if (!camera){
        self.getLogger().pfatal("cannot select camera: "
            +"'"+a_name+"' camera not found\n"
        );
    }
    camera.select();
}

ObjectViewer.prototype.drawScene = function(
    a_picking)
{
    var self    = this;
    var camera  = self.getCamera();

    if (!camera){
        self.pfatal("cannot get camera\n");
    }

    camera.drawScene(a_picking);
}

ObjectViewer.prototype.tick = function()
{
    var self = this;
    var gl   = self.getGl();

    if (!gl){
        self.getLogger().pfatal('empty gl');
    }

    // calculate timings
    self.m_tick_time_ms_cur = new Date().getTime();
    if (self.m_tick_time_ms_prev){
        self.m_tick_time_ms_elapsed
            = self.m_tick_time_ms_cur
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

    //self.handleKeys();
    self.drawScene();

    // store last tick time
    self.m_tick_time_ms_prev = self.m_tick_time_ms_cur;
}

ObjectViewer.prototype.handleMouseDown = function(
    a_ev)
{
    var self    = this;
    var camera  = self.getCamera();

    self.m_mouse_down = 1;
    camera.handleMouseDown(a_ev);
}

ObjectViewer.prototype.handleMouseUp = function(
    a_ev)
{
    var self    = this;
    var camera  = self.getCamera();

    self.m_mouse_down = 0;
    camera.handleMouseUp(a_ev);
}

ObjectViewer.prototype.parseMouseMoveEvent = function(
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

ObjectViewer.prototype.handleMouseMoveCamera = function(
    a_ev)
{
    var self = this;

    var ev = self.parseMouseMoveEvent(a_ev);
    yaw     -= ev.move.x/2;
    pitch   -= ev.move.y/2;
}

ObjectViewer.prototype.handleMouseMoveScene = function(
    a_ev)
{
    var self = this;

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

ObjectViewer.prototype.handleMouseMove = function(
    a_ev)
{
    var self = this;

    if (self.m_pointer_locked){
        self.handleMouseMoveCamera(a_ev);
    } else {
        self.handleMouseMoveScene(a_ev);
    }
}

ObjectViewer.prototype.getGl = function()
{
    var self = this;
    var ret  = self.m_gl;
    if (!ret){
        self.getLogger().pfatal('getGl failed');
    }
    return ret;
}

ObjectViewer.prototype.getCanvas = function()
{
    var self = this;
    return self.m_canvas;
}

ObjectViewer.prototype.getShaders = function()
{
    var self = this;
    return self.m_shaders;
}

ObjectViewer.prototype.getFps = function()
{
    var self = this;
    return self.m_fps;
}

// ---------------- module_info ----------------

var module_info = {
    'type':         ObjectViewer.prototype.getObjectType(),
    'objectCreate': function(){
        return new ObjectViewer();
    }
};
module_register(module_info);

