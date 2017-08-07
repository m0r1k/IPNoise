function Cub(
    a_viewer)
{
    var self = this;

    self.m_viewer       = a_viewer;
    self.m_pos          = vec3.create();
    self.m_pos_home     = vec3.create();
    self.m_rot          = quat4.create();
    self.m_tex          = vec3.create();
    self.m_mag          = 0;
    self.m_spot         = null;
    self.m_cube_buf     = {};
    self.m_scale        = 1;
    self.m_is_selected  = -1;
    self.m_size         = 1;
    self.m_selectedFace = 1;
    self.m_tweens       = [];

    self.m_logger = new Logger();
    self.m_logger.setPrefix('cub');

    // Set up buffers
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
    self.m_cube_buf.vert.bind = function(shader){
        var attrs = shader.getAttrs();

        gl.bindBuffer(
            gl.ARRAY_BUFFER,
            self.m_cube_buf.vert
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

    var ind = [
        0 ,1 ,2 ,        0 ,2 ,3 ,        // -X face
        4 ,5 ,6 ,        4 ,6 ,7 ,        // +X face
        8 ,9 ,10,        8 ,10,11,        // -Y face
        12,13,14,        12,14,15,        // +Y face
        16,17,18,        16,18,19,        // -Z face
        20,21,22,        20,22,23,        // +Z face
    ];
    self.m_cube_buf.ind = gl.createBuffer();
    gl.bindBuffer(
        gl.ELEMENT_ARRAY_BUFFER,
        self.m_cube_buf.ind
    );
    gl.bufferData(
        gl.ELEMENT_ARRAY_BUFFER,
        new Uint16Array(ind),
        gl.STATIC_DRAW
    );
    self.m_cube_buf.ind.itemCount = 36;
}

Cub.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    self.m_logger = a_logger;
}

Cub.prototype.getLogger = function()
{
    var self = this;
    return self.m_logger;
}

Cub.prototype.getGl = function()
{
    var self = this;
    var gl   = self.m_viewer.getGl();
    return gl;
}

Cub.prototype.setPos = function(
    a_arg)
{
    var self = this;
    if (undefined == a_arg){
        self.getLogger().pfatal("empty argument");
    }
    // self.getLogger().pdebug(1, "MORIK setPos:", a_arg);
    self.m_pos  = a_arg;
}

Cub.prototype.setPosHome = function(
    a_arg)
{
    var self        = this;
    self.m_pos_home = a_arg;
}

Cub.prototype.setRot = function(
    a_arg)
{
    var self    = this;
    self.m_rot  = a_arg;
}

Cub.prototype.setTex = function(
    a_arg)
{
    var self    = this;
    self.m_tex  = a_arg;
}

Cub.prototype.setMag = function(
    a_arg)
{
    var self    = this;
    self.m_mag  = a_arg;
}

Cub.prototype.setSpot = function(
    a_arg)
{
    var self    = this;
    self.m_spot = a_arg;
}

Cub.prototype.setScale = function(
    a_arg)
{
    var self        = this;
    self.m_scale    = a_arg;
}

Cub.prototype.setSize = function(
    a_arg)
{
    var self        = this;
    self.m_size     = a_arg;
}

Cub.prototype.move = function(dir)
{
    var self = this;

    var axis = self.m_selectedFace >> 1;
    var sign = (self.m_selectedFace & 1) ? -1 : 1;

    self.getLogger().pdebug(1, "move", {
        'axis': axis,
        'sign': sign
    });

    // Is there already a tween for this?
    var oldtw;
    for (var i = 0; i < self.m_tweens.length; i++){
        if ("move" == self.m_tweens[i].type){
            oldtw = self.m_tweens.splice(i, 1)[0];
            break;
        }
    }

    var oldPos   = self.m_pos;
    var newPos   = [0,0,0];
    newPos[axis] = dir * sign * (1/self.m_size);

    console.log("1/self.m_size: "+1/self.m_size);

    quat4.multiplyVec3(
        self.m_rot,
        newPos
    );
    vec3.add(
        newPos,
        oldtw
            ? oldtw.to
            : oldPos
    );

    self.m_tweens.push({
        type:       "move",
        from:       oldPos,
        to:         newPos,
        startTime:  new Date().getTime(),
        duration:   10000,
    });
}

Cub.prototype.rotate = function(dir)
{
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
}

// ---------------- api ----------------

Cub.prototype.animate = function()
{
    var self    = this;
    var nowTime = new Date().getTime();
    var res;

    for (var i = 0; i < self.m_tweens.length; i++){
        var tw = self.m_tweens[i];
        var complete = (nowTime - tw.startTime) / tw.duration;
        if (complete < 0){
            complete = 0;
        }
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
                self.setPos(res);
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
                self.getLogger().pdebug(50, "animate scale", {
                    'tw.from':  tw.from,
                    'tw.to':    tw.to,
                    'complete': complete
                });
                self.m_scale = tw.from
                    + complete * (tw.to - tw.from);
                console.log("Scale " + self.m_scale);
                break;
        }
    }

    return (self.m_tweens.length > 0);
}

Cub.prototype.draw = function(a_pick_color)
{
    var self = this;

    if (undefined != a_pick_color){
        var shader      = g_shaders.use('pick');
        var attrs       = shader.getAttrs();
        var uniforms    = shader.getUniforms();

        gl.uniformMatrix4fv(
            uniforms.uPMatrix,
            false,
            pMatrix
        );

        gl.bindBuffer(
            gl.ELEMENT_ARRAY_BUFFER,
            self.m_cube_buf.ind
        );

        // setup attributes offsets in buffer
        self.m_cube_buf.vert.bind(shader);

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
            gl.TRIANGLES,
            self.m_cube_buf.ind.itemCount,
            gl.UNSIGNED_SHORT,
            0
        );
    } else {
        var shader      = g_shaders.use('wood');
        var attrs       = shader.getAttrs();
        var uniforms    = shader.getUniforms();

        gl.uniformMatrix4fv(
            uniforms.uPMatrix,
            false,
            pMatrix
        );

        gl.bindBuffer(
            gl.ELEMENT_ARRAY_BUFFER,
            self.m_cube_buf.ind
        );

        // setup attributes offsets in buffer
        self.m_cube_buf.vert.bind(shader);

        gl.uniform1f(
            uniforms.uScale,
            self.m_size
        );

//        self.getLogger().plog(1, "MORIK tigra", {
//            'self.m_rot':   self.m_rot,
//            'self.m_pos':   self.m_pos
//        });

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
            self.m_cube_buf.ind.itemCount,
            gl.UNSIGNED_SHORT,
            0
        );
    }
}

Cub.prototype.select = function()
{
    var self = this;
    self.m_is_selected = 0.1;
}

Cub.prototype.deSelect = function()
{
    var self = this;
    self.m_is_selected = -1;
}

Cub.prototype.handleKeys = function()
{
    var self = this;

    if (currentlyPressedKeys[87]){
        // W
        self.move(-1);
    }
    if (currentlyPressedKeys[83]){
        // S
        self.move(+1);
    }
    if (currentlyPressedKeys[39]){
        // Right cursor key
        self.rotate(+1);
    }
    if (currentlyPressedKeys[37]){
        // Left cursor key
        self.rotate(-1);
    }
}

// -------------------------------------

