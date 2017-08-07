function ShaderWood(
    a_viewer)
{
    var self = new Shader(a_viewer, 'wood');

    self.m_logger = new Logger();
    self.m_logger.setPrefix('shader: '+self.m_name);

    var gl = self.m_viewer.getGl();

    var res;
    var woodFrag = getShader(gl, "wood-fs");
    var woodVert = getShader(gl, "wood-vs");

    self.m_program = gl.createProgram();
    gl.attachShader(self.m_program, woodVert);
    gl.attachShader(self.m_program, woodFrag);
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
            'aTexCoord',
//            'aFaceIndex',
            'aSpot'
        ],
        uniforms:   [
            'uPMatrix',
            'uMVMatrix',
            'uScale',
            'uOffset',
            'uSelected'
        ]
    });
}

registerShader("ShaderWood");

