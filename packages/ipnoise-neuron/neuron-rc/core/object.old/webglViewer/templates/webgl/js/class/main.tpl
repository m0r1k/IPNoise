// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseWebglViewer",   // TODO
    $.ipnoise.IPNoiseWebglObject,   // remove Webgl
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // TODO
        self.element.attr('object_id', 'viewer');

        // store to global
        g_viewer = self.element;

        // prevent double click to select text
        self.element.disableSelection();

        var args = {
            'viewer':   self
        };

        // copy args
        for (var key in a_args){
            args[key] = a_args[key];
        }

        self._super(args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglViewer');

        // get window info
        var win         = $(window);
        var win_heigh   = win.height();
        var win_width   = win.width();

        // prepare elements
        self.m_inner = $('<div>');
        self.m_inner.attr('class', 'inner');
        self.m_inner.attr('style', 'text-align: center;');

        self.m_canvas = $('<canvas>');
        self.m_canvas.hide();
        self.m_canvas.attr('id',       'webGLCanvas');
        self.m_canvas.attr('width',    '800');
        self.m_canvas.attr('height',   '600');
        self.m_canvas.appendTo(self.m_inner);

        // create start button
        self.start_button = $('<button>');
        self.start_button.css('position',  'absolute');
        self.start_button.css('top',  win_heigh/2 - 50 + 'px');
        self.start_button.css('left', win_width/2 - 50 + 'px');
        self.start_button.html('Start');
        self.start_button.on('click',   function(){
            g_viewer.callWidgetMethod('enterFullscreen');
        });
        self.start_button.appendTo(self.m_inner);

        // fill element
        self.m_inner.appendTo(self.element);

        self.m_shaders                  = undefined;
        self.m_gl                       = undefined;
        self.m_camera                   = undefined;
        self.m_fps                      = 25;
        self.m_pointer_locked           = 0;
        self.m_webgl_inited             = 0;

        self.m_tick_time_ms_cur         = 0;
        self.m_tick_time_ms_prev        = 0;
        self.m_tick_time_ms_elapsed     = 0;

        self.m_cur_pressed_keys         = {};

        $(document).on("webkitfullscreenchange"
            +" mozfullscreenchange"
            +" fullscreenchange",
            function(a_ev){
                g_viewer.callWidgetMethod(
                    'fullScreenChanged',
                    a_ev
                );
            }
        );

        $(document).on("webkitpointerlockchange"
            +" mozpointerlockchange"
            +" pointerlockchange",
            function(a_ev){
                g_viewer.callWidgetMethod(
                    'pointerLockChanged',
                    a_ev
                );
            }
        );

        // do not forget return self!
        return self;
    },

    handleKeys: function(){
        var self    = this;
        var camera  = self.getCurCamera();
    
        if (camera){
            camera.callWidgetMethod(
                'handleKeys',
                self.m_cur_pressed_keys
            );
        }
    },

    handleKeyDown: function(
        a_ev)
    {
        var code   = a_ev.keyCode;
        var widget = g_viewer;

        getLogger().pdebug(10,
            "key down, code: '"+code+"'"
        );

        widget.callWidgetMethod(
            'setCurrentlyPressedKey',
            {
                'code':     code,
                'pressed':  true
            }
        );
    },

    handleKeyUp: function(
        a_ev)
    {
        var code    = a_ev.keyCode;
        var widget  = g_viewer;

        getLogger().pdebug(10,
            "key up, code: '"+code+"'"
        );

        widget.callWidgetMethod(
            'setCurrentlyPressedKey',
            {
                'code':     code,
                'pressed':  false
            }
        );
    },

    handleMouseWheel: function(a_ev){
        var self   = this;
        var camera = self.getCurCamera();
        if (camera){
            camera.callWidgetMethod(
                'handleMouseWheel',
                a_ev
            );
        }
        // Don't scroll the page as well
        return false;
    },

    handleScrambleButton: function(scramble){
        // randomize/solve puzzle
        cube.scramble(scramble);
    },

    handleGapRadio: function(size){
        // gap size 0 - none, 1 - small, 2 - big
        cube.setGap(size);
    },

    pointerLock: function(a_el){
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
    },

    pointerUnLock: function(a_el){
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
    },

    togglePointer: function(){
        var self = this;

        var state = self.m_pointer_locked;
        if (state){
            self.pointerUnLock();
        } else {
            self.pointerLock();
        }
    },

    isFullScreen: function(a_el){
        var self = this;

        var el = a_el;
        if (!el){
            el = document;
        }
        var ret = el.fullscreenEnabled
            || el.mozFullscreenEnabled
            || el.webkitIsFullScreen;

        return ret;
    },

    enterFullscreen: function(){
        var self    = this;
        var canvas  = self.m_canvas;
        var el      = canvas.get(0);

        if (el.webkitRequestFullScreen){
            el.webkitRequestFullScreen(
                Element.ALLOW_KEYBOARD_INPUT
            );
        } else {
            el.mozRequestFullScreen();
        }
    },

    getViewportWidth: function(){
        var self = this;
        return self.m_gl.viewportWidth;
    },

    getViewportHeight: function(){
        var self = this;
        return self.m_gl.viewportHeight;
    },

    init_webgl: function(){
        var self = this;

        if (self.m_webgl_inited){
            return;
        }

        var doc    = $(document);
        var canvas = self.m_canvas;

        self.m_gl                   = WebGLUtils.setupWebGL(canvas.get(0));
        self.m_gl.viewportWidth     = canvas.get(0).width;
        self.m_gl.viewportHeight    = canvas.get(0).height;

        canvas.mousedown(self.handleMouseDown);
        doc.mouseup(self.handleMouseUp);
        canvas.mousemove(self.handleMouseMove);
        canvas.get(0).onmousewheel = function(a_ev){
            var target = $(a_ev.target);
            // TODO use target here
            g_viewer.callWidgetMethod(
                'handleMouseWheel',
                a_ev
            );
        }

        doc.keyup(self.handleKeyUp);
        doc.keydown(self.handleKeyDown);

        self.initObjects();
    },

    fullScreenChanged: function(a_ev){
        var self          = this;
        var is_fullscreen = self.isFullScreen();
        var canvas        = self.getCanvas();

        if (is_fullscreen){
            canvas.attr(
                'style',
                'text-align: center;'
            );

            canvas.show();
            canvas.old_width     = canvas.width;
            canvas.old_height    = canvas.height;
            canvas.get(0).width  = window.innerWidth;
            canvas.get(0).height = window.innerHeight;

            self.init_webgl();
        } else {
            self.start_button.html('Full screen');

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
    },

    pointerLockChanged: function(a_ev){
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
    },

    initObjects: function(){
        var self = this;
        self.createShaders();
    },

    createShaders:  function(){
        var self = this;
        createWidget({
            'class_name':   'IPNoiseWebglShaders',
            'args':         {
                'viewer': self.element,
            },
            'success':      function(a_data, a_widgets){
                if (1 != a_widgets.length){
                    self.getLogger().pfatal(
                        "invalid widgets count:"
                        +" '"+a_widgets.length+"'",
                        {
                            'a_data':       a_data,
                            'a_widgets':    a_widgets
                        }
                    );
                }

                var shaders_el = a_widgets[0];
                self.m_shaders = shaders_el;

                self.getCreateCamera();

                // mark as inited
                self.m_webgl_inited = 1;

                // start draw
                timer_draw();
            }
        });
    },

    getCurCamera: function(){
        var self   = this;
        var camera = self.m_camera;
        return camera;
    },

    getCreateCamera: function(){
        var self = this;
        getCreateCamera({
            'params':   {},
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

                self.m_camera = a_widgets[0];
                self.m_camera.callWidgetMethod('watchStart');
            },
            'failed':   function(){
                self.getLogger().pfatal("cannot init camera");
            }
        });
    },

/*
    createCube: function(){
        var self = this;

        self.getLogger().pdebug(1, "createCube\n");

        createWidget({
            'class_name':   'IPNoiseObjectShop',
            'args':         {
                'viewer':       self.element,
                'pos':          vec3.create(
                    [-0.8, 0, 0]
                ),
                'size':         3,
            },
            'success':      function(a_data, a_widgets){
                if (1 != a_widgets.length){
                    self.getLogger().pfatal(
                        "invalid widgets count:"
                        +" '"+a_widgets.length+"'",
                        {
                            'a_data':       a_data,
                            'a_widgets':    a_widgets
                        }
                    );
                }

                var cube_el = a_widgets[0];
                self.addForDraw(cube_el);

                self.createCurObject();
            }
        });
    },
*/

    drawScene: function(
        a_picking)
    {
        var self   = this;
        var camera = self.getCurCamera();
        if (camera){
            camera.callWidgetMethod('drawScene', a_picking);
        }
    },

    tick: function(){
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

        self.handleKeys();
        self.drawScene();

        // store last tick time
        self.m_tick_time_ms_prev = self.m_tick_time_ms_cur;
    },

    handleMouseDown: function(a_ev){
        var camera = g_viewer.callWidgetMethod('getCurCamera');
        if (camera){
            camera.callWidgetMethod('handleMouseDown', a_ev);
        }
    },

    handleMouseUp: function(
        a_ev)
    {
        var camera = g_viewer.callWidgetMethod('getCurCamera');
        if (camera){
            camera.callWidgetMethod('handleMouseUp', a_ev);
        }
    },

    parseMouseMoveEvent: function(
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
    },

    handleMouseMoveCamera: function(
        a_ev)
    {
        var self = this;

        var ev = self.parseMouseMoveEvent(a_ev);
        yaw     -= ev.move.x/2;
        pitch   -= ev.move.y/2;
    },

    handleMouseMoveScene: function(
        a_ev)
    {
        var widget     = g_viewer;
        var canvas     = widget.callWidgetMethod('getCanvas');
        var mouse_down = widget.callWidgetMethod(
            'isMouseDown'
        );

        var last_mouse_X;
        var last_mouse_Y;

        if (!mouse_down){
            return;
        }

        pos_x = a_ev.offsetX
            ?   (a_ev.offsetX)
            :   a_ev.pageX - canvas.offsetLeft;

        pos_y = a_ev.offsetY
            ?   (a_ev.offsetY)
            :   a_ev.pageY - canvas.offsetTop;

        last_mouse_X = pos_x;
        last_mouse_Y = pos_y;

        widget.callWidgetMethod(
            'setLastMouseX',
            last_mouse_X
        );

        widget.callWidgetMethod(
            'setLastMouseY',
            last_mouse_Y
        );
    },

    handleMouseMove: function(
        a_ev)
    {
        var self = this;

        //if (self.m_pointer_locked){
        //    self.handleMouseMoveCamera(a_ev);
        //} else {
        //    self.handleMouseMoveScene(a_ev);
        //}
    },

    getCurrentlyPressedKeys: function(){
        var self = this;
        return currentlyPressedKeys;
    },

    getGl: function(){
        var self = this;
        var ret  = self.m_gl;
        if (!ret){
            self.getLogger().pfatal('getGl failed');
        }
        return ret;
    },

    initKeyInfo: function(a_code){
        var self = this;
        if (!self.m_cur_pressed_keys[a_code]){
            self.m_cur_pressed_keys[a_code] = {}
        }
        self.m_cur_pressed_keys[a_code].time_press    = 0;
        self.m_cur_pressed_keys[a_code].time_elapsed  = 0;
        // all keys in 'inc' will be incremented
        self.m_cur_pressed_keys[a_code].inc           = {};
        return self.m_cur_pressed_keys[a_code];
    },

    setCurrentlyPressedKey: function(a_args){
        var self    = this;
        var code    = a_args.code;
        var pressed = a_args.pressed;

        var cur_time_ms = new Date().getTime();

        var key_info = self.m_cur_pressed_keys[code];
        if (!key_info){
            key_info = self.initKeyInfo(code);
        }

        if (pressed){
            if (!key_info.time_press){
                // first press, mark start time
                key_info.time_press      = cur_time_ms;
                key_info.time_elapsed    = 0;
                key_info.time_last_check = cur_time_ms;
            } else {
                // cont press
                var delta  = cur_time_ms - key_info.time_last_check;
                key_info.time_elapsed += delta;
                // update handlers variable
                for (var key in key_info.inc){
                    key_info.inc[key] += delta;
                }
                key_info.time_last_check = cur_time_ms;
            }
        } else {
            delete self.m_cur_pressed_keys[code];
        }
    },

    getCanvas: function(){
        var self = this;
        return self.m_canvas;
    },

    getShaders: function(){
        var self = this;
        return self.m_shaders;
    },

    getFps: function(){
        var self = this;
        return self.m_fps;
    },
});

