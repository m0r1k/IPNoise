// vim:syntax=javascript tabstop=4 expandtab

// setup vertex buffer and attributes offsets
function vertex_array_bind(
    a_gl,
    a_vert_buff,
    a_shader)
{
    var self  = this;
    var gl    = a_gl;
    var attrs = a_shader.callWidgetMethod('getAttrs');

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

$.widget(
    "ipnoise.IPNoiseWebglObject",
    $.ipnoise.IPNoiseObject,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        if (undefined == a_args.viewer){
            getLogger().pfatal("missing argument: 'viewer'",
                a_args
            );
        }
        self.m_viewer = a_args.viewer;

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglObject');

        self.worldVertexPositionBuffer       = undefined;
        self.worldVertexTextureCoordBuffer   = undefined;
        self.textures                        = [];
        self.ready                           = 0;

        self.m_pos          = vec3.create();
        self.m_pos_home     = vec3.create();
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

        //self.initCubeBuffers();
        self.initAtomBuffers();

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
        var args = {
            'setup_attribute':  1,
            'handled':          0
        };
        copy_args(args, a_args);
        self._super(a_key, a_val, args);
    },

    getViewer: function(){
        var self = this;
        return self.m_viewer;
    },

    initCubeBuffers: function(){
        // Set up buffers
        var self    = this;
        var gl      = self.getGl();
        var s       = 0.5;
        var verts   = [
            // Pos,         normal,         tex uv,     face index
            -s,-s,-s,       -1, 0, 0,       0,0,        0,     // -X face
            -s,-s, s,       -1, 0, 0,       1,0,        0,
            -s, s, s,       -1, 0, 0,       1,1,        0,
            -s, s,-s,       -1, 0, 0,       0,1,        0,
             s, s, s,        1, 0, 0,       0,0,        1,     // +X face
             s,-s, s,        1, 0, 0,       1,0,        1,
             s,-s,-s,        1, 0, 0,       1,1,        1,
             s, s,-s,        1, 0, 0,       0,1,        1,
            -s,-s,-s,        0,-1, 0,       0,0,        2,     // -Y face
             s,-s,-s,        0,-1, 0,       1,0,        2,
             s,-s, s,        0,-1, 0,       1,1,        2,
            -s,-s, s,        0,-1, 0,       0,1,        2,
             s, s, s,        0, 1, 0,       0,0,        3,    // +Y face
             s, s,-s,        0, 1, 0,       1,0,        3,
            -s, s,-s,        0, 1, 0,       1,1,        3,
            -s, s, s,        0, 1, 0,       0,1,        3,
            -s,-s,-s,        0, 0,-1,       0,0,        4,    // -Z face
            -s, s,-s,        0, 0,-1,       1,0,        4,
             s, s,-s,        0, 0,-1,       1,1,        4,
             s,-s,-s,        0, 0,-1,       0,1,        4,
             s, s, s,        0, 0, 1,       0,0,        5,    // +Z face
            -s, s, s,        0, 0, 1,       1,0,        5,
            -s,-s, s,        0, 0, 1,       1,1,        5,
             s,-s, s,        0, 0, 1,       0,1,        5
        ];

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

        var index = [
            0 ,1 ,2 ,        0 ,2 ,3 ,        // -X face
            4 ,5 ,6 ,        4 ,6 ,7 ,        // +X face
            8 ,9 ,10,        8 ,10,11,        // -Y face
            12,13,14,        12,14,15,        // +Y face
            16,17,18,        16,18,19,        // -Z face
            20,21,22,        20,22,23,        // +Z face
        ];
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
    },

    initAtomBuffers: function(){
        // Set up buffers
        var self    = this;
        var gl      = self.getGl();
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
    },

    getGl: function(){
        var self = this;
        return self.m_viewer.callWidgetMethod('getGl');
    },
    getShaders: function(){
        var self = this;
        return self.m_viewer.callWidgetMethod('getShaders');
    },
   
    // ---------------- api ----------------

    animate: function(){
        var self    = this;
        var nowTime = new Date().getTime();
        var res;

        for (var i = 0; i < self.m_tweens.length; i++){
            var tw = self.m_tweens[i];
            var complete = (nowTime - tw.startTime) / tw.duration;
            if (complete < 0){
                complete = 0;
            }

            complete = 1; // MORIK TODO XXX remove me

            if (complete >= 1){
                complete = 1;
                self.m_tweens.splice(i, 1);
            }

            switch (tw.type){
                case "move":
                    self.getLogger().pdebug(50, "animate move", {
                        'tw.from':  tw.from,
                        'tw.to':    tw.to,
                        'complete': complete
                    });
                    res = vec3.create();
                    vec3.lerp(
                        tw.from,
                        tw.to,
                        complete,
                        res
                    );
                    self.updateObject({
                        'params': {
                            'pos':  [
                                res[0],
                                res[1],
                                res[2]
                            ]
                        }
                    });
                    break;

                case "rotate":
                    self.getLogger().pdebug(50, "animate rotate", {
                        'tw.from':  tw.from,
                        'tw.to':    tw.to,
                        'complete': complete
                    });
                    res = vec3.create();
                    quat4.slerp(
                        tw.from,
                        tw.to,
                        complete,
                        res
                    );
                    self.rotate(res);
                    break;

                case "scale":
                    self.getLogger().pdebug(
                        50,
                        "animate scale",
                        {
                            'tw.from':  tw.from,
                            'tw.to':    tw.to,
                            'complete': complete
                        }
                    );
                    self.m_scale = tw.from
                        + complete * (tw.to - tw.from);
                    console.log("Scale " + self.m_scale);
                    break;
            }
        }

        return (self.m_tweens.length > 0);
    },

    draw_old: function(a_pick_color){
        var self        = this;
        var shaders     = self.getShaders();
        var gl          = self.getGl();
        var mvMatrix    = self.getMvMatrix();
        var pMatrix     = self.getPMatrix();
        var zMatrix     = self.getZMatrix();

        if (undefined != a_pick_color){
            var shader_class = 'IPNoiseWebglShaderPick';
            var shader       = shaders.callWidgetMethod(
                'use',
                shader_class
            );
            if (!shader){
                getLogger().pfatal("cannot init shader: '"+shader_class+"'");
            }

            var attrs       = shader.callWidgetMethod('getAttrs');
            var uniforms    = shader.callWidgetMethod('getUniforms');

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

            // convert virtual coordinates to real
            var x = self.m_pos[X] * self.m_size;
            var y = self.m_pos[Y] * self.m_size;
            var z = self.m_pos[Z] * self.m_size;
            var pos = vec3.create([x, y, z]);

            var localMatrix = mat4.fromRotationTranslation(
                self.m_rot,
                pos
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
                gl.TRIANGLES,
                self.m_cube_buf.index.itemCount,
                gl.UNSIGNED_SHORT,
                0
            );
        } else {
            var shader_class = 'IPNoiseWebglShaderWood';
            var shader       = shaders.callWidgetMethod(
                'use',
                shader_class
            );
            if (!shader){
                getLogger().pfatal("cannot init shader: '"+shader_class+"'");
            }

            var attrs       = shader.callWidgetMethod('getAttrs');
            var uniforms    = shader.callWidgetMethod('getUniforms');

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

            //  self.getLogger().plog(1, "MORIK tigra", {
            //      'self.m_rot':   self.m_rot,
            //      'self.m_pos':   self.m_pos
            //  });

            // convert virtual coordinates to real
            var x = self.m_pos[X] * self.m_size;
            var y = self.m_pos[Y] * self.m_size;
            var z = self.m_pos[Z] * self.m_size;
            var pos = vec3.create([x, y, z]);

            var localMatrix = mat4.fromRotationTranslation(
                self.m_rot,
                pos
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

            // add spot
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                self.m_spot
            );
            gl.vertexAttribPointer(
                attrs.aSpot,
                4,
                gl.FLOAT,
                false,
                0,
                0
            );

            gl.drawElements(
                gl.TRIANGLES,
                self.m_cube_buf.index.itemCount,
                gl.UNSIGNED_SHORT,
                0
            );
        }
    },

    draw_new: function(a_pick_color){
        var self        = this;
        var shaders     = self.getShaders();
        var gl          = self.getGl();

        var camera      = g_viewer.callWidgetMethod('getCurCamera');
        var mvMatrix    = camera.callWidgetMethod('getMvMatrix');
        var pMatrix     = camera.callWidgetMethod('getPMatrix');
        var zMatrix     = camera.callWidgetMethod('getZMatrix');
        var pos         = JSON_STR_TO_VEC3(self.option('object_pos'));
        var view_pos    = JSON_STR_TO_VEC3(self.option('object_view_pos'));

        if (!mvMatrix){
            self.getLogger().pfatal("missing 'mvMatrix'");
        }

        if (!pMatrix){
            self.getLogger().pfatal("missing 'pMatrix'");
        }

        if (!zMatrix){
            self.getLogger().pfatal("missing 'zMatrix'");
        }

        if (!pos){
            self.getLogger().pfatal("missing 'pos'");
        }

        if (!view_pos){
            self.getLogger().pfatal("missing 'view_pos'");
        }

        if (undefined != a_pick_color){
            var shader_class = 'IPNoiseWebglShaderAtom';
            var shader       = shaders.callWidgetMethod(
                'use',
                shader_class
            );
            if (!shader){
                getLogger().pfatal("cannot init shader: '"+shader_class+"'");
            }

            var attrs       = shader.callWidgetMethod('getAttrs');
            var uniforms    = shader.callWidgetMethod('getUniforms');

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
                pos
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
            var shader_class = 'IPNoiseWebglShaderAtom';
            var shader       = shaders.callWidgetMethod(
                'use',
                shader_class
            );
            if (!shader){
                getLogger().pfatal("cannot init shader: '"+shader_class+"'");
            }

            var attrs       = shader.callWidgetMethod('getAttrs');
            var uniforms    = shader.callWidgetMethod('getUniforms');

            // draw view vector
            if (1){
                var x1 = pos[X];
                var y1 = pos[Y];
                var z1 = pos[Z];

                var x2 = view_pos[X];
                var y2 = view_pos[Y];
                var z2 = view_pos[Z];

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
//                var shader_class = 'IPNoiseWebglShaderAtom';
//                var shader       = shaders.callWidgetMethod(
//                    'use',
//                    shader_class
//                );
//                if (!shader){
//                    getLogger().pfatal("cannot init shader: '"+shader_class+"'");
//                }

//                var attrs       = shader.callWidgetMethod('getAttrs');
//                var uniforms    = shader.callWidgetMethod('getUniforms');

//                gl.uniformMatrix4fv(
//                    uniforms.uPMatrix,
//                    false,
//                    pMatrix
//                );

//                gl.uniformMatrix4fv(
//                    uniforms.uZMatrix,
//                    false,
//                    zMatrix
//                );
 
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

//                gl.uniform1f(
//                    uniforms.uScale,
//                    self.m_size
//                );

                // rotate if need
                var localMatrix = mat4.fromRotationTranslation(
                    self.m_rot,
                    vec3.create([ 0, 0, 0 ]) //pos
                );

//                mat4.scale(
//                    localMatrix,
//                    [
//                        self.m_scale,
//                        self.m_scale,
//                        self.m_scale
//                    ]
//                );
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

//                gl.uniform3f(
//                    uniforms.uOffset,
//                    self.m_tex[0],
//                    self.m_tex[1],
//                    self.m_tex[2]
//                );

//                // add information about
//                // selection
//                gl.uniform1f(
//                    uniforms.uSelected,
//                    self.m_is_selected
//                );

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

            // --------------------------------------------------

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
                pos
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
    },

    draw: function(a_pick_color){
        var self = this;
        self.draw_new(a_pick_color);
    },

    select: function(){
        var self = this;
        self._super();
    },

    deSelect: function(){
        var self = this;
        self._super();
    },

    handleMouseWheel: function(a_ev){
        var self     = this;
        var move     = a_ev.wheelDelta/240;

        self.getLogger().pdebug(
            1,
            "object: '"+self.option('object_id')+"'"
            +" handleMouseWheel: '"+move+"'"
        );
    },

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;
        var skip_super  = 0;

        if ('objects.changed' == type){
            var objects = a_event.objects;
            if (objects.viewver){
                skip_super = 1;
            }            
        }

        // maybe super event?
        self._super(a_event);
    },

});

