/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Window base class implementation
 *  Aug, 2009.
 */

var top_zindex = 10;

// clean node and childs
function cleanNode (node){
    if (!node)
        return;
    // remove childs
    while (node.firstChild)
        node.removeChild(node.firstChild);
    // remove node
    var parentNode = node.parentNode;
    if (parentNode){
        parentNode.removeChild(node);
    }
};

function getWin(icon)
{
    var node = icon;
    while (node){
        var id = node.getAttribute('id');
        if (id){
            var win = getClassByGuiId(id);
            if (win){
                return win;
            }
        }
        node = node.parentNode;
    }
    return null;
}

function closeWin(element)
{
    var win = getWin(element);
    if (win){
        win.close();
    }
}

function maximizeWin(close_icon)
{
    var win = getWin(close_icon);
    if (win){
        win.maximize();
    }
}

function minimizeWin(close_icon)
{
    var win = getWin(close_icon);
    if (win){
        win.minimize();
    }
}

var last_ev_pos = null;

function WindowClass(args)
{
    var self    = AbstractClass();

    var ev      = args.ev;
    var context = args.context;
    if (!context){
        perror("Missing argument 'context'");
        return null;
    }

    self.ready4left_resize  = 0;
    self.ready4right_resize = 0;
    self.ready4top_resize   = 0;
    self.ready4bootm_resize = 0;

    self.resize_border      = 5;

    var context_doc = context.document;

    var hide                = 0;
    var hide_head           = 0;
    var disable_resize      = 0;
    var disable_move        = 0;
    var hide_instead_close  = 0;

    if (args["hide"]){
        hide = 1;
    } 
    if (args["hide-head"]){
        hide_head = 1;
    }
    if (args["disable-resize"]){
        disable_resize = 1;
    }
    if (args["disable-move"]){
        disable_move = 1;
    }
    if (args["hide_instead_close"]){
        hide_instead_close = 1;
    }

    var tmpl_win = TemplateWindowClass({
        "id":         self.getClassGuiId(),
        "hide-head":  hide_head
    });

    self.focus = function(){
        top_zindex++;
        tmpl_win.main_div.css("z-index", top_zindex); 
    }

    self.showContextMenu = function(){
        // alert("тут будет контекстное меню");
        return true;
    }

    self.setTitle = function (new_title){
        tmpl_win.setTitle(new_title);
    }

    self.maximize = function()
    {
        tmpl_win.maximize();
    }

    self.minimize = function()
    {
        tmpl_win.minimize();
    }

    self.resize = function(x, y)
    {
        tmpl_win.setSize(x, y);
        tmpl_win.refresh();
    }
    self.hide = function()
    {
        tmpl_win.main_div.hide();//css("display", "none");
    }

    self.checkResize = function(ev)
    {
        if (resizeObject){
            // now we in resize mode 
            return;
        }

        var mousePos    = mouseCoords(ev);
        var win         = tmpl_win.main_div;
        var winPos      = self.getAbsPos();

        if (mousePos.x    > (winPos.x + win.width())
            && mousePos.x < (winPos.x + win.width() + self.resize_border))
        {
            self.ready4right_resize = 1;
            self.setCursor("e-resize");
        } else {
            self.ready4right_resize = 0;
        }
        if (mousePos.y    > (winPos.y + win.height())
            && mousePos.y < (winPos.y + win.height() + self.resize_border))
        {
            self.ready4bottom_resize = 1;
            self.setCursor("s-resize");
        } else {
            self.ready4bottom_resize = 0;
        }
        
        if (   !self.ready4left_resize && !self.ready4right_resize
            && !self.ready4top_resize  && !self.ready4bottom_resize)
        { 
            self.setCursor("default");
        }
    }

    self.refresh = function(){
        if (!disable_resize){
            tmpl_win.main_div.mouseover(function(ev){
                self.checkResize(ev);
            });
            tmpl_win.main_div.mouseout(function(ev){
                self.ready4left_resize  = "0";
                self.ready4right_resize = "0";
                self.ready4top_resize   = "0";
                self.ready4bootm_resize = "0";
                self.setCursor("default");
            }); 
            makeResizeble(tmpl_win.main_div);
        }
        if (!disable_move){
            var div_window = context_doc.getElementById(self.getClassGuiId());
            makeDraggable(div_window.getElementsByTagName("DIV")[0]);
        }
        tmpl_win.onClose("closeWin(this);");
        tmpl_win.onMinimize("minimizeWin(this);");
        tmpl_win.onMaximize("maximizeWin(this);");
        tmpl_win.refresh(); 
    }
    self.unhide = function()
    {
        tmpl_win.main_div.show();//css("display", "block");
        self.refresh();
    }
    self.close = function()
    {
        if (hide_instead_close){
            return self.hide();
        }
        tmpl_win.main_div.empty();
        cleanClass(tmpl_win);
        cleanClass(self);
    }

    self.setFixedSize = function(x, y){
        tmpl_win.setFixedSize(x, y);
        tmpl_win.refresh();
    }

    self.setSize = function(x, y){
        if (   x != "auto"
            && y != "auto")
        {
            // todo if will "auto" ?
            tmpl_win.setSize(x, y);
            tmpl_win.refresh();
        }
    }

    self.setBodyElement = function (data){
        tmpl_win.setBody(data);
    }

    self.setInnerHtml = function (html){
        var data = $("<div>"+html+"</div>");
        self.setBodyElement(data);
    }

    self.setAbsPos = function (x, y){
        var win = tmpl_win.main_div;
        win.css("left", x);
        win.css("top",  y);
    }

    self.getAbsPos = function (){
        var res = tmpl_win.main_div.offset();
        return {
            x: res.left,
            y: res.top,
        };
    }

    self.getSize = function(){
        return {
          x: tmpl_win.main_div.css("clientWidth").replace(/\D/g, ""),
          y: tmpl_win.main_div.css("clientHeight").replace(/\D/g, "")
        };
    }

    self.setCursor = function(val){
        tmpl_win.main_div.css("cursor", val);
    }

    if (ev && ev.target){
        // set position near of event
        last_ev_pos = mouseCoords(ev);
        //last_ev_pos = getPosition(ev.target);
    }

    if (last_ev_pos){
        self.setAbsPos(last_ev_pos.x, last_ev_pos.y);
    }

    if (hide){
        self.hide();
    }

    tmpl_win.main_div.appendTo("#body");

    self.refresh();

    return self;
};

