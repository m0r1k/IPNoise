$.widget(
    "ipnoise.IPNoiseWebglShaderAtom",
    $.ipnoise.IPNoiseWebglShader,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // TODO
        self.element.attr('object_id', 'shader_atom');

        // prevent double click to select text
        self.element.disableSelection();

        if (undefined == a_args.viewer){
            getLogger().pfatal("missing argument: 'viewer'");
        }

        self._super('atom', a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglShaderAtom');

        var gl = self.getGl();

        var res;
        var atomFrag = getShader(gl, "atom-fs");
        var atomVert = getShader(gl, "atom-vs");

        self.m_program = gl.createProgram();
        gl.attachShader(self.m_program, atomVert);
        gl.attachShader(self.m_program, atomFrag);
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
                'uZMatrix',
                'uPickColor'
            ]
        });

        // do not forget return self!
        return self;
    }
});

