// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

<!-- temporary close js -->
</script>

<script id="atom-vs" type="x-shader/x-vertex">
    precision   mediump     float;

    attribute   vec3        aPosition;
    // attribute   float       aFaceIndex;

    uniform     mat4        uMVMatrix;
    uniform     mat4        uPMatrix;
    uniform     mat4        uZMatrix;

    // varying     float       vFaceIndex;

    void main(void){
        // Set the size of the point
        gl_PointSize = 6.0;

        gl_Position = uPMatrix
            * uMVMatrix
            * uZMatrix
            * vec4(aPosition,1.0);

        // vFaceIndex = aFaceIndex;
    }
</script>

<script id="atom-fs" type="x-shader/x-fragment">
    precision   mediump     float;

    uniform vec3            uPickColor;
    // varying float           vFaceIndex;

    void main(void){
        gl_FragColor = vec4(
            uPickColor/255.0,
            1.0
        );

        // gl_FragColor = vec4(
        //    uPickColor/255.0,
        //    1.0
        // ) + vec4(
        //        0.5,
        //        0.5,
        //        vFaceIndex/255.0,
        //        0
        //    );
    }

</script>


<!-- back to js -->
<script type="text/javascript">

