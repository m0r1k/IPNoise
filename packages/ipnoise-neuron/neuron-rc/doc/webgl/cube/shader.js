var g_max_shaders_index = -1;

function Shader(
    a_viewer,
    a_name)
{
    var self = this;

    if (undefined == a_viewer){
        g_logger.pfatal("missing argument: 'viewer'");
    }

    if (undefined == a_name){
        g_logger.pfatal("missing argument: 'name'");
    }

    self.m_viewer       = a_viewer;
    self.m_name         = a_name;
    self.m_attrs        = {};
    self.m_uniforms     = {};
    self.m_program      = null;
    self.m_logger       = null;
};

Shader.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    self.m_logger = a_logger;
}

Shader.prototype.getLogger = function()
{
    var self = this;
    return self.m_logger;
}

Shader.prototype.getGl = function()
{
    var self = this;
    var gl   = self.m_viewer.getGl();
    return gl;
}

Shader.prototype.getName = function()
{
    var self = this;
    return self.m_name;
}

Shader.prototype.setupAttributes = function()
{
    var self = this;
    var gl   = self.getGl();

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

Shader.prototype.initLocations = function(
    a_args)
{
    var self = this;
    var gl   = self.getGl();

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
            +" for shader: '"+self.getName()+"'"
        );
    }
}

Shader.prototype.getProgram = function()
{
    var self = this;
    return self.m_program;
}

Shader.prototype.getAttrs = function()
{
    var self = this;
    return self.m_attrs;
}

Shader.prototype.getUniforms = function()
{
    var self = this;
    return self.m_uniforms;
}

Shader.prototype.use = function()
{
    var self = this;
    var gl   = self.getGl();
    gl.useProgram(self.m_program);
    self.setupAttributes();
}

