/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Drag and Drop class implementation
 *  Jun, 2009
 *
 */

var dragObject   = null; 
var resizeObject = null;
var mouseOffset  = null; 

context.onmousemove = mouseMove; 
context.onmouseup   = mouseUp; 
 
Object = null; 
var mouseOffset = null; 

function mouseCoords(ev){ 
    if(ev.pageX || ev.pageY){ 
        return {x:ev.pageX, y:ev.pageY}; 
    } 
    return { 
        x:ev.clientX + context.body.scrollLeft - context.body.clientLeft, 
        y:ev.clientY + context.body.scrollTop  - context.body.clientTop
    }; 
}

function getMouseOffset(target, ev)
{
    ev = ev || context.window.event; 
 
    var docPos    = getPosition(target); 
    var mousePos  = mouseCoords(ev); 
    return {x:mousePos.x - docPos.x, y:mousePos.y - docPos.y}; 
} 
 
function getPosition(e)
{
    var left = 0; 
    var top  = 0; 
 
    while (e.offsetParent){ 
        left += e.offsetLeft; 
        top  += e.offsetTop; 
        e     = e.offsetParent; 
    } 
 
    left += e.offsetLeft; 
    top  += e.offsetTop; 
 
    return {x:left, y:top}; 
} 
 
function mouseMove(ev)
{
    ev           = ev || context.window.event; 
    var mousePos = mouseCoords(ev); 
 
    if (dragObject){ 
        dragObject.style.position = 'absolute'; 
        dragObject.style.top      = mousePos.y - mouseOffset.y; 
        dragObject.style.left     = mousePos.x - mouseOffset.x; 
        return false; 
    }

    if (!resizeObject){
        return false;
    }
    var id = resizeObject.getAttribute("id");
    if (!id){
        return false;
    }
    var win = getClassByGuiId(id);
    if (!win){
        return false;
    }
    var win_pos  = win.getAbsPos();
    var win_size = win.getSize();

    if (win.ready4right_resize){
        var new_width = mousePos.x - win_pos.x;
        win_size.x    = new_width;
        win.setSize(win_size.x, win_size.y);
    }
    if (win.ready4bottom_resize){
        var new_height = mousePos.y - win_pos.y;
        win_size.y     = new_height;
        win.setSize(win_size.x, win_size.y);
    }
} 

function mouseUp(ev)
{
    ev           = ev || context.window.event; 
    var mousePos = mouseCoords(ev); 

    if (dragObject){
        var id  = dragObject.getAttribute("id");
        var win = getClassByGuiId(id);
        if (win){
            win.setAbsPos(
                dragObject.style.left, dragObject.style.top
            );
            win.setCursor("default");
        }
    }

    if (resizeObject){
        var id  = resizeObject.getAttribute("id");
        var win = getClassByGuiId(id);
        if (win){
             win.ready4left_resize  = 0;
             win.ready4right_resize = 0;
             win.ready4top_resize   = 0;
             win.ready4bootm_resize = 0;       
        }
    }
    dragObject   = null; 
    resizeObject = null; 

} 
 
function makeDraggable(item)
{
    if(!item) return; 
    item.onmousedown = function(ev){
        dragObject  = this.parentNode;
        mouseOffset = getMouseOffset(this.parentNode, ev);
    };
} 

function makeResizeble(item)
{ 
    if (!item) return; 
    item.mousedown(function(ev){ 
        var id = this.getAttribute("id");
        if (!id){
            return;
        }
        var win = getClassByGuiId(id);
        if (!win){
            return;
        }
        // set focus
        win.focus();
        if (   !win.ready4left_resize && !win.ready4right_resize
            && !win.ready4top_resize  && !win.ready4bottom_resize)
        {
            return;
        }
 
        resizeObject = this;
        return;
    });
} 




