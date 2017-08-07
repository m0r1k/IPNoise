// ---------------- static ----------------

var g_logger = document.g_logger;

// ---------------- ObjectShaderAtom ----------------

function ObjectShaderAtom()
{
    var self = this;
}

ObjectShaderAtom.prototype               = new ObjectShader;
ObjectShaderAtom.prototype.getObjectType = function()
{
    return 'core.object.shader.atom';
}

ObjectShaderAtom.prototype.v_reset = function(
    a_args)
{
    var self    = this;
    var args    = {
        'document': undefined
    };
    copy_args(args, a_args);

    if (!args.document){
        self.getLogger().pfatal(
            "missing argument: 'document'"
        );
    }

    // init super
    ObjectShader.prototype.v_reset.apply(self, [ args ]);
}

ObjectShaderAtom.prototype.compile = function()
{
    var self        = this;
    var viewer      = self.getViewer();
    var gl          = viewer.getGl();
    var atomFrag    = getShader(gl, "atom-fs");
    var atomVert    = getShader(gl, "atom-vs");
    var res;

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
}

// ---------------- module ----------------

var module_info = {
    'type':         ObjectShaderAtom.prototype.getObjectType(),
    'objectCreate': function(a_args){
        return new ObjectShaderAtom(a_args);
    }
};
module_register(module_info);

