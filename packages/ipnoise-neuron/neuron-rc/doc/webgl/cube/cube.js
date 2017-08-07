// http://code.google.com/p/glmatrix/wiki/Usage

function Cube(
    a_viewer,
    a_size,
    a_pos)
{
    var self = this;

    self.m_viewer = a_viewer;

    var size = a_size;
    if (size == undefined){
        size = 2;
    }

    self.m_size                 = size;
    self.m_scale                = 0.85/size
    self.m_selectedCube         = -1;
    self.m_tweens               = [];
    self.m_cubeStates           = [];

    self.m_added_for_draw       = 0;

    self.m_logger = new Logger();
    self.m_logger.setPrefix('cube');

    self.m_pos = a_pos;

    var s = (size - 1)/2;
    var i = 0;

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

        spots = [];
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
        var pick_color = i * 256*256*256/(size*size*size - 1);

        var cub = new Cub(self.m_viewer);
        cub.setPos(
            vec3.create([
                self.m_pos[X] + (x-s)/size,
                self.m_pos[Y] + (y-s)/size,
                self.m_pos[Z] + (z-s)/size
            ])
        );
        cub.setPosHome(vec3.create(
            [(x-s)/size, (y-s)/size, (z-s)/size]
        ));
        cub.setRot(quat4.create(
            [0, 0, 0, 1]
        ));
        cub.setTex(vec3.create(
            [(x-s)/size, (y-s)/size, (z-s)/size]
        ));
        cub.setMag(mag);
        cub.setSpot(spotBuf);
        cub.setScale(self.m_scale);
        cub.setSize(1/size);

        self.m_cubeStates.push(cub);
    }
}

Cube.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    self.m_logger = a_logger;
}

Cube.prototype.getLogger = function()
{
    var self = this;
    return self.m_logger;
}

Cube.prototype.getGl = function()
{
    var self = this;
    var gl   = self.m_viewer.getGl();
    return gl;
}

Cube.prototype.setPos = function(
    a_v3_pos)
{
    var self = this;
    g_logger.pdebug(1, "morik setpos", a_v3_pos);
    self.m_pos = a_v3_pos;
}

Cube.prototype.getSelectedCube = function()
{
    var self = this;
    return self.m_selectedCube;
}

Cube.prototype.scramble = function(scramble)
{
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

            for (var i = 0; i < self.m_cubeStates.length; i++){
                // Select a random, orthogonal orientation
                // This algorithm probably sucks, but it works.
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
}

Cube.prototype.setGap = function(
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
}

// ---------------- api ----------------

Cube.prototype.animate = function()
{
}


Cube.prototype.draw = function(
    a_pick_color)
{
    var self = this;

    // do not clear screen
    // gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    if (!self.m_added_for_draw){
        self.m_added_for_draw = 1;
        for (var i = 0; i < self.m_cubeStates.length; i++){
            addForDraw(self.m_cubeStates[i]);
        }
    }
}

Cube.prototype.select   = function(){}
Cube.prototype.deSelect = function(){}

// -------------------------------------

