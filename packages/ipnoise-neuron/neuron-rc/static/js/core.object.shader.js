// ---------------- static ----------------

var g_logger            = document.g_logger;
var g_max_shaders_index = -1;

// ---------------- Shader ----------------

function ObjectShader()
{
    var self = this;

    self.m_attrs        = {};
    self.m_uniforms     = {};
    self.m_program      = undefined;
}

ObjectShader.prototype               = new Object;
ObjectShader.prototype.getObjectType = function()
{
    return 'core.object.shader';
}

ObjectShader.prototype.v_reset = function(
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
    Object.prototype.v_reset.apply(self, [ args ]);
}

ObjectShader.prototype.setupAttributes = function()
{
    var self    = this;
    var viewer  = self.getViewer();
    var gl      = viewer.getGl();

    for (var i = 1; i <= g_max_shaders_index; i++){
        // OpenGL requires attribute zero
        // to be enabled otherwise
        // it will not render anything
        gl.disableVertexAttribArray(i);
    }

    for (var attr_name in self.m_attrs){
        var index = self.m_attrs[attr_name];
        gl.enableVertexAttribArray(index);
    }
}

ObjectShader.prototype.initLocations = function(
    a_args)
{
    var self    = this;
    var viewer  = self.getViewer();
    var gl      = viewer.getGl();

    var args = {
        attrs:      [],
        uniforms:   []
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var attrs    = args.attrs;
    var uniforms = args.uniforms;

    for (var i = 0; i < attrs.length; i++){
        var cur_attr = attrs[i];
        var index    = gl.getAttribLocation(
            self.m_program,
            cur_attr
        );
        if (0 > index){
            self.getLogger().pfatal(
                 "cannot get attr: '"+cur_attr+"' location"
            );
        }

        // store attr index to shader
        self.m_attrs[cur_attr] = index;

        if (index > g_max_shaders_index){
            g_max_shaders_index = index;
        }

        // enable attribute
        gl.enableVertexAttribArray(index);

        self.getLogger().pdebug(
            10,
               "enabling attr: '"+cur_attr+"'"
            +" location ("+index+")"
        );
    }

    // uniforms
    for (var i = 0; i < uniforms.length; i++){
        var cur_uniform = uniforms[i];
        var obj         = gl.getUniformLocation(
            self.m_program,
            cur_uniform
        );
        if (!obj){
            self.getLogger().pfatal(
             "cannot get uniform: '"+cur_uniform+"'"
                +" location,"
                +" for shader: '"+self.getName()+"'"
            );
        }

        self.m_uniforms[cur_uniform] = obj;

        self.getLogger().pdebug(
            10,
            "found uniform: '"+cur_uniform+"'"
            +" location,"
            +" for shader: '"+self.getObjectName()+"'"
        );
    }
}

ObjectShader.prototype.getProgram = function()
{
    var self = this;
    return self.m_program;
}

ObjectShader.prototype.getAttrs = function()
{
    var self = this;
    return self.m_attrs;
}

ObjectShader.prototype.getUniforms = function()
{
    var self = this;
    return self.m_uniforms;
}

ObjectShader.prototype.use = function()
{
    var self    = this;
    var viewer  = self.getViewer();
    var gl      = viewer.getGl();

    gl.useProgram(self.m_program);
    self.setupAttributes();
}

// ---------------- module ----------------

var module_info = {
    'type':         ObjectShader.prototype.getObjectType(),
    'objectCreate': function(a_args){
        return new ObjectShader(a_args);
    }
};
module_register(module_info);

