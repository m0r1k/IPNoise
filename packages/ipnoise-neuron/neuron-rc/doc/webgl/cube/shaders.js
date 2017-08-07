var registered_shaders = {};

// static
function getShader(
    a_gl,
    a_id)
{
    var shader;
    var shader_script;
    var str = "";
    var gl  = a_gl;
    var k;

    shader_script = document.getElementById(a_id);
    if (!shader_script){
        return null;
    }

    k = shader_script.firstChild;
    while (k){
        if (3 == k.nodeType){
            str += k.textContent;
        }
        k = k.nextSibling;
    }

    if (shader_script.type == "x-shader/x-fragment"){
        shader = gl.createShader(gl.FRAGMENT_SHADER);
    } else if (shader_script.type == "x-shader/x-vertex"){
        shader = gl.createShader(gl.VERTEX_SHADER);
    } else {
        return null;
    }

    gl.shaderSource(shader, str);
    gl.compileShader(shader);

    res = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
    if (!res){
        shader = null;
    }

    return shader;
}

// static
function registerShader(
    a_class_name)
{
    if (registered_shaders[a_class_name]){
        g_logger.pwarn(
            "shader: '"+name+"' already registered"
        );
    } else {
        registered_shaders[a_class_name] = null;
        g_logger.pdebug(
            1,
            "shader: '"+a_class_name+"' was registered"
        );
    }
}

function Shaders(
    a_viewer)
{
    var self = this;

    self.m_viewer               = a_viewer;
    self.m_cur_shader           = null;
    self.m_cur_shader_name      = null;

    self.m_logger = new Logger();
    self.m_logger.setPrefix('shaders');

    for (var class_name in registered_shaders){
        var shader_name;
        var shader;

        shader = registered_shaders[class_name];
        if (!shader){
            shader = window[class_name](self.m_viewer);
            if (!shader){
                g_logger.pfatal(
                    "cannot init shader: '"+class_name+"'"
                );
            }
            registered_shaders[class_name] = shader;
        }
        shader_name = shader.getName();
        self.getLogger().pinfo(
            "init shader: '"+shader_name+"'"
        );
    }
}

Shaders.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    self.m_logger = a_logger;
}

Shaders.prototype.getLogger = function()
{
    var self = this;
    return self.m_logger;
}

Shaders.prototype.use = function(
    a_shader_name)
{
    var self    = this;
    var shader  = registered_shaders[a_shader_name];
    var program = null;

    if (!shader){
        self.getLogger().pfatal(
            "can not use shader: '"+a_shader_name+"',"
            +" shader not found"
        );
    }

    self.m_cur_shader       = shader;
    self.m_cur_shader_name  = a_shader_name;

    shader.use();

    return shader;
}

