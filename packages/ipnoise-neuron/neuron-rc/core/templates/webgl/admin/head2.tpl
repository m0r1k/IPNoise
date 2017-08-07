<script id="shader-vs" type="x-shader/x-vertex">
    attribute vec3 aVertexPosition;
    attribute vec4 aVertexColor;

    uniform mat4 uMVMatrix;
    uniform mat4 uPMatrix;

    varying vec4 vColor;

    void main(void){
        gl_Position     = uPMatrix * uMVMatrix * vec4(
            aVertexPosition,
            1.0
        );
        vColor          = aVertexColor;
        gl_PointSize    = 5.0;
    }
</script>

<script id="shader-fs" type="x-shader/x-fragment">
    precision mediump float;
    varying vec4 vColor;

    void main(void) {
        gl_FragColor = vColor;
    }
</script>

<script type="text/javascript">

    var gl;
    function initGL(canvas) {
        try {
            gl = canvas.getContext("experimental-webgl");
            gl.viewportWidth = canvas.width;
            gl.viewportHeight = canvas.height;
        } catch (e) {
        }
        if (!gl) {
            alert("Could not initialise WebGL, sorry :-(");
        }
    }


    function getShader(gl, id) {
        var shaderScript = document.getElementById(id);
        if (!shaderScript) {
            return null;
        }

        var str = "";
        var k = shaderScript.firstChild;
        while (k) {
            if (k.nodeType == 3) {
                str += k.textContent;
            }
            k = k.nextSibling;
        }

        var shader;
        if (shaderScript.type == "x-shader/x-fragment") {
            shader = gl.createShader(gl.FRAGMENT_SHADER);
        } else if (shaderScript.type == "x-shader/x-vertex") {
            shader = gl.createShader(gl.VERTEX_SHADER);
        } else {
            return null;
        }

        gl.shaderSource(shader, str);
        gl.compileShader(shader);

        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            alert(gl.getShaderInfoLog(shader));
            return null;
        }

        return shader;
    }


    var shaderProgram;

    function initShaders(){
        var res;
        var fragmentShader  = getShader(gl, "shader-fs");
        var vertexShader    = getShader(gl, "shader-vs");

        shaderProgram = gl.createProgram();
        gl.attachShader(shaderProgram, vertexShader);
        gl.attachShader(shaderProgram, fragmentShader);
        gl.linkProgram(shaderProgram);

        res = gl.getProgramParameter(
            shaderProgram,
            gl.LINK_STATUS
        );
        if (!res){
            alert("Could not initialise shaders");
        }

        gl.useProgram(shaderProgram);

        // vertexPositionAttribute
        shaderProgram.vertexPositionAttribute
            = gl.getAttribLocation(
                shaderProgram,
                "aVertexPosition"
            );
        gl.enableVertexAttribArray(
            shaderProgram.vertexPositionAttribute
        );

        // vertexColorAttribute
        shaderProgram.vertexColorAttribute
            = gl.getAttribLocation(
                shaderProgram,
                "aVertexColor"
            );
        gl.enableVertexAttribArray(
            shaderProgram.vertexColorAttribute
        );

        // matrixs
        shaderProgram.pMatrixUniform = gl.getUniformLocation(
            shaderProgram,
            "uPMatrix"
        );
        shaderProgram.mvMatrixUniform = gl.getUniformLocation(
            shaderProgram,
            "uMVMatrix"
        );
    }

    var mvMatrix = mat4.create();
    var pMatrix  = mat4.create();

    function setMatrixUniforms() {
        gl.uniformMatrix4fv(
            shaderProgram.pMatrixUniform,
            false,
            pMatrix
        );
        gl.uniformMatrix4fv(
            shaderProgram.mvMatrixUniform,
            false,
            mvMatrix
        );
    }

    var item_size;
    var triangle_buffer;
    var triangle_strip_buffer;

    var buffer_lines_pos;
    var buffer_lines_colors;

    function initBuffers() {
        // first
        triangle_buffer = gl.createBuffer();
        gl.bindBuffer(
            gl.ARRAY_BUFFER,
            triangle_buffer
        );
        var vertices = [
            -0.1, -0.1,  0.0,
             0.0,  0.1,  0.0,
             0.1, -0.1,  0.0,
             0.2,  0.1,  0.0,
             0.4, -0.1,  0.0
        ];
        gl.bufferData(
            gl.ARRAY_BUFFER,
            new Float32Array(vertices),
            gl.STATIC_DRAW
        );
        item_size                = 3;
        triangle_buffer.itemSize = item_size;
        triangle_buffer.numItems = vertices.length/item_size;

        // second
        triangle_strip_buffer = gl.createBuffer();
        gl.bindBuffer(
            gl.ARRAY_BUFFER,
            triangle_strip_buffer
        );
        vertices = [
             1.0,  1.0,  0.0,
            -1.0,  1.0,  0.0,
             1.0, -1.0,  0.0,
            -1.0, -1.0,  0.0
        ];
        gl.bufferData(
            gl.ARRAY_BUFFER,
            new Float32Array(vertices),
            gl.STATIC_DRAW
        );
        item_size                      = 3;
        triangle_strip_buffer.itemSize = item_size;
        triangle_strip_buffer.numItems = vertices.length/item_size;

        // third (pos)
        buffer_lines_pos = gl.createBuffer();
        gl.bindBuffer(
            gl.ARRAY_BUFFER,
            buffer_lines_pos
        );
        vertices = [
            -1.0,  0.0,  0.0,
             1.0,  0.0,  0.0,

            -1.0,  1.0,  0.0,
             1.0,  1.0,  0.0,

            -1.0,  2.0,  0.0,
             1.0,  2.0,  0.0
        ];
        gl.bufferData(
            gl.ARRAY_BUFFER,
            new Float32Array(vertices),
            gl.STATIC_DRAW
        );
        item_size                   = 3;
        buffer_lines_pos.itemSize   = item_size;
        buffer_lines_pos.numItems   = vertices.length/item_size;
        // third (colors)
        buffer_lines_colors = gl.createBuffer();
        gl.bindBuffer(
            gl.ARRAY_BUFFER,
            buffer_lines_colors
        );
        vertices = [
             1.0,  0.0,  0.0,   1.0,
             1.0,  0.0,  0.0,   1.0,

             1.0,  1.0,  0.0,   1.0,
             1.0,  1.0,  0.0,   1.0,

             1.0,  0.5,  0.0,   1.0,
             1.0,  0.5,  0.0,   1.0,
        ];
        gl.bufferData(
            gl.ARRAY_BUFFER,
            new Float32Array(vertices),
            gl.STATIC_DRAW
        );
        item_size                    = 4;
        buffer_lines_colors.itemSize = item_size;
        buffer_lines_colors.numItems = vertices.length/item_size;

    }

    function drawScene() {
        gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        mat4.perspective(
            45,
            gl.viewportWidth / gl.viewportHeight,
            0.1,
            100.0,
            pMatrix
        );

        mat4.identity(mvMatrix);

        // first
        if (0){
            mat4.translate(mvMatrix, [-1.5, 0.0, -7.0]);
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                triangle_buffer
            );
            gl.vertexAttribPointer(
                shaderProgram.vertexPositionAttribute,
                triangle_buffer.itemSize,
                gl.FLOAT,
                false,
                0,
                0
            );
            setMatrixUniforms();
            gl.drawArrays(
                // gl.TRIANGLES,
                gl.TRIANGLE_STRIP,
                0,
                triangle_buffer.numItems
            );
        }

        // second
        if (0){
            mat4.translate(mvMatrix, [-1.5, 0.0, -7.0]);
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                triangle_strip_buffer
            );
            gl.vertexAttribPointer(
                shaderProgram.vertexPositionAttribute,
                triangle_strip_buffer.itemSize,
                gl.FLOAT,
                false,
                0,
                0
            );
            setMatrixUniforms();
            gl.drawArrays(
                // gl.TRIANGLES,
                gl.TRIANGLE_STRIP,
                0,
                triangle_strip_buffer.numItems
            )
        }

        // third
        if (1){
            mat4.translate(mvMatrix, [-1.5, 0.0, -7.0]);

            // vertexes
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                buffer_lines_pos
            );
            gl.vertexAttribPointer(
                shaderProgram.vertexPositionAttribute,
                buffer_lines_pos.itemSize,
                gl.FLOAT,
                false,
                0,
                0
            );

            gl.lineWidth(5.0);

            // colors
            gl.bindBuffer(
                gl.ARRAY_BUFFER,
                buffer_lines_colors
            );
            gl.vertexAttribPointer(
                shaderProgram.vertexColorAttribute,
                buffer_lines_colors.itemSize,
                gl.FLOAT,
                false,
                0,
                0
            );
        }

        setMatrixUniforms();

        gl.drawArrays(
            gl.LINE_STRIP,
            0,
            buffer_lines_pos.numItems
        );
    }



    function webGLStart() {
        var canvas = document.getElementById("canvas_main");
        initGL(canvas);
        initShaders();
        initBuffers();

        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.enable(gl.DEPTH_TEST);

        drawScene();
    }

</script>

