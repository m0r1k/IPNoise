// ---------------- static ----------------

var g_logger = document.g_logger;

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
        return  undefined;
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
        return undefined;
    }

    gl.shaderSource(shader, str);
    gl.compileShader(shader);

    res = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
    if (!res){
        shader = undefined;
    }

    return shader;
}

// ---------------- ObjectShaders ----------------

function ObjectShaders()
{
    var self = this;

    self.m_shaders = {};
}

ObjectShaders.prototype                 = new Object;
ObjectShaders.prototype.getObjectType   = function()
{
    return 'core.object.shaders';
}

ObjectShaders.prototype.v_reset = function(
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

ObjectShaders.prototype.use = function(
    a_object_type)
{
    var self    = this;
    var shader  = undefined;
    var viewer  = self.getViewer();

    if (!viewer){
        self.getLogger().pfatal("cannot get viewer\n",
            self
        );
    }

    shader = self.m_shaders[a_object_type];
    if (!shader){
        shader = object_create(a_object_type);
        shader.setViewer(viewer);
        shader.compile();
        self.m_shaders[a_object_type] = shader;
    }

    shader.use();

    return shader;
}

// ---------------- module ----------------

var module_info = {
    'type':         ObjectShaders.prototype.getObjectType(),
    'objectCreate': function(a_args){
        return new ObjectShaders(a_args);
    }
};
module_register(module_info);

