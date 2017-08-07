// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

var g_viewer = undefined;

function timer_draw(){
    var widget = g_viewer;
    var fps    = widget.callWidgetMethod('getFps');

    widget.callWidgetMethod('tick');
    //requestAnimationFrame(timer_draw);

    setTimeout(
        timer_draw,
        1000 / fps
    );
}

