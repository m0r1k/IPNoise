$.widget(
    "ipnoise.IPNoiseWebglShaderWood",
    $.ipnoise.IPNoiseWebglShader,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // TODO
        self.element.attr('object_id', 'shader_wood');

        // prevent double click to select text
        self.element.disableSelection();

        if (undefined == a_args.viewer){
            getLogger().pfatal("missing argument: 'viewer'");
        }

        self._super('wood', a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglShaderWood');

        var gl = self.getGl();

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
                // 'aFaceIndex',
                'aSpot'
            ],
            uniforms:   [
                'uPMatrix',
                'uMVMatrix',
                'uZMatrix',
                'uScale',
                'uOffset',
                'uSelected'
            ]
        });

        // do not forget return self!
        return self;
    }
});

