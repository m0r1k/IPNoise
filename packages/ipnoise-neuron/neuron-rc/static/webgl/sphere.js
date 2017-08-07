
function start3d(texture_image)
{
    var use_texture = 0;

    var w = texture_image.width;
    var h = texture_image.height;

    var texinfo1 = new Pre3d.TextureInfo();
    texinfo1.image = texture_image;
    texinfo1.u0 = 0;
    texinfo1.v0 = 0;
    texinfo1.u1 = 0;
    texinfo1.v1 = h;
    texinfo1.u2 = w;
    texinfo1.v2 = h;

    var texinfo2 = new Pre3d.TextureInfo();
    texinfo2.image = texture_image;
    texinfo2.u0 = 0;
    texinfo2.v0 = 0;
    texinfo2.u1 = w;
    texinfo2.v1 = h;
    texinfo2.u2 = w;
    texinfo2.v2 = 0;

    function selectTexture(quad_face, quad_index, shape) {
        // Each face is two triangles, the newly triangulated triangles last.
        renderer.texture = quad_index < 6 ? texinfo1 : texinfo2;
        //renderer.texture = texinfo1;
        return false;
    }

    var black           = new Pre3d.RGBA(0, 0, 0, 1);
    var screen_canvas   = document.getElementById('canvas');
    var renderer        = new Pre3d.Renderer(screen_canvas);

    var sphere = Pre3d.ShapeUtils.makeSphere(10,30,20);

    if (use_texture){
        // use texture
        renderer.quad_callback  = selectTexture;
        renderer.fill_rgba      = null;
    }

    function draw() {
        if (!use_texture){
            renderer.fill_rgba = new Pre3d.RGBA(0,255,0,0.2);
        }
        renderer.bufferShape(sphere);

        renderer.ctx.setFillColor(0, 0, 0, 1);
        renderer.drawBackground();

        renderer.drawBuffer();
        renderer.emptyBuffer();
    }

    renderer.camera.focal_length = 2.5;

    // Have the engine handle mouse / camera movement for us.
    DemoUtils.autoCamera(renderer, 0, 0, -30, 0.40, -1.06, 0, draw);

    draw();


/*
    setInterval(function(){
        var camera_state = {
            rotate_x:   0,
            rotate_y:   -1.06,
            rotate_z:   0,
            x:          0,
            y:          0,
            z:          -30
        };

        var ct = renderer.camera.transform;
        ct.reset();
        ct.rotateZ(camera_state.rotate_z);
        ct.rotateY(camera_state.rotate_y);
        ct.rotateX(camera_state.rotate_x);
        ct.translate(camera_state.x, camera_state.y, camera_state.z);

        renderer.camera.focal_length -= 0.001;

        draw();
    }, 100);
*/
}


window.addEventListener('load', function()
{
    var img     = new Image();
    img.onload  = function (){ start3d(img); };
    img.src     = '/static/images/128x128/dreams_01.png';
}, false);

