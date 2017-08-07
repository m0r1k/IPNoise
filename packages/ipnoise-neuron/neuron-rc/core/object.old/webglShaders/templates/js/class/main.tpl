// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

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

