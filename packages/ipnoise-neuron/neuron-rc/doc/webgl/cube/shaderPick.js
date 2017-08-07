function ShaderPick(
    a_viewer)
{
    var self = new Shader(a_viewer, 'pick');

    self.m_logger = new Logger();
    self.m_logger.setPrefix('shader: '+self.m_name);

    var gl = self.m_viewer.getGl();

    var res;
    var pickFrag = getShader(gl, "pick-fs");
    var pickVert = getShader(gl, "pick-vs");

    self.m_program = gl.createProgram();
    gl.attachShader(self.m_program, pickVert);
    gl.attachShader(self.m_program, pickFrag);
    gl.linkProgram(self.m_program);

    res = gl.getProgramParameter(
        self.m_program,
        gl.LINK_STATUS
    );
    if (!res){
        self.getLogger().pfatal(
            "cannot init shader: '"+self.m_name+"'"
        );
    }

    self.initLocations({
        attrs:      [
            'aPosition',
            // 'aNormal',
            // 'aTexCoord',
            // 'aFaceIndex',
            // 'aSpot'
        ],
        uniforms:   [
            'uPMatrix',
            'uMVMatrix',
            'uPickColor'
        ]
    });
}

registerShader("ShaderPick");

