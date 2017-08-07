/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Message window class implementation
 *  Aug, 2009.
 */

function viewer(context, url)
{
    var win = WindowClass({context: context});
    win.setInnerHtml("<img src='"+url+"'/>");
    win.focus();
}

function filter(body)
{
    var new_body = body;

//  temporary disabled by morik@
//    new_body = new_body.replace(/(http:\/\/.*)/,
//        '<a style="text-decoration: underline; cursor: pointer;" onclick="viewer(window, \'$1\');">$1</a>'
//    );
    new_body = new_body.replace(/[\r\n]/g, "<br>\n");

    return new_body;
}

var last_typing_event_time = 0;

// возвращает html строку для сообщения
function getMsgLine(msg)
{
    var class_name = "";
    if (msg.getType() == "outcoming"){
        class_name = "msgOutcoming";
    } else if (msg.getType() == "incoming"){
        class_name = "msgIncoming";
    } else if (msg.getType() == "system"){
        class_name = "msgSystem";
    } else {
        class_name = "msgUnknown";
    }

    var msg_id = msg.getId();
    if (!msg_id){
        msg_id = "0";
    }

    var msg_date = new Date(msg.getTime() * 1000);
    var year  = msg_date.getFullYear();
    var month = msg_date.getMonth();
    var day   = msg_date.getDate();
    var hour  = msg_date.getHours();
    var min   = msg_date.getMinutes();
    var sec   = msg_date.getSeconds();
    var msec  = msg_date.getMilliseconds();

    year  = text_int_align(year,  4);
    month = text_int_align(month, 2);
    day   = text_int_align(day,   2);
    hour  = text_int_align(hour,  2);
    min   = text_int_align(min,   2);
    sec   = text_int_align(sec,   2);
    msec  = text_int_align(msec,  3);

    var data_as_string = hour+":"+min+":"+sec;
    var html = '<div class="row msg '+class_name+' msg_id_'+msg_id+'">'
        +'<div class="col">'+data_as_string+'</div>'
        +'<div class="col" style="width: 100%">'+msg.getBody()+'</div>'
        +'</div>';

    return html;
}

function getWinObjByElement(element)
{
    var win_obj = null;
    var node = element;
    while (node){
        var id = node.getAttribute("id");
        if (id && /^InI_/.test(id)){
            win_obj = getClassByGuiId(id);
            break;
        }
        node = node.parentNode;
    };
    return win_obj;
}

function showHistory (element, ev)
{
    var src_chatid = "0";
    var dst_chatid = "0";

    // search window
    var win_obj = getWinObjByElement(element);
    if (win_obj && win_obj.huid){
        var hist_win = historyWindowClass({
            "context":      window,
            "huid":         win_obj.huid,
            "hide":         0,
        });
        hist_win.setSize("700", "500");
        hist_win.setAbsPos(200, 250);
        hist_win.focus();
        hist_win.setTitle("История разговоров с "+win_obj.name);
        return false;
    }
    return true;
}

function keyPress(element, ev)
{
    var keycode;
    if (window.event){
        keycode = window.event.keyCode;
    } else if (ev) {
        keycode = ev.which;
    } else {
        return true;
    }

    var src_chatid = "0";
    var dst_chatid = "0";
 
    var node = element;
    while (node){
        if (node.tagName == "DIV"){
            if (node.className){
                var do_break = 0;
                var regexp = /chatbody_srcid_(\d+)/;
                var res = regexp.exec(node.className);
                if (res && res[1]){
                    src_chatid = res[1];
                    do_break   = 1;
                }
                regexp = /chatbody_dstid_(\d+)/;
                res = regexp.exec(node.className);
                if (res && res[1]){
                    dst_chatid = res[1];
                    do_break   = 1;
                }
                if (do_break){
                    break;
                }
            }
        }
        node = node.parentNode;
    }
    if (keycode >= 32
     && keycode != 13
     && keycode != 88
     && !ev.ctrlKey
     && !ev.altKey)
    {
        var cur_date    = new Date;
        var unixtime_ms = cur_date.getTime();
        var unixtime    = parseInt(unixtime_ms / 1000);

        if ((unixtime - last_typing_event_time) >= 3){
            // send typing event
            var win_obj = getWinObjByElement(element);
            if (win_obj){
                var msg = msgClass();
                msg.setBody("");
                msg.setSrcChatId(src_chatid);
                msg.setDstChatId(dst_chatid);
                msg.setType("outcoming");
                win_obj.sendMsg(msg);
                cleanClass(msg);
                last_typing_event_time = unixtime;
            }
        }
    }

    if (!ev.ctrlKey){
        return true;
    }

    if ((keycode == 13 || keycode == 88)
        && element.value)
    {
        // 'Enter' or 'x'
        // search window
        var win_obj = getWinObjByElement(element);
        if (win_obj){
            // all good, send
            var msg = msgClass();
            msg.setBody(element.value);
            msg.setSrcChatId(src_chatid);
            msg.setDstChatId(dst_chatid);
            msg.setType("outcoming");
            var res = win_obj.sendMsg(msg);
            cleanClass(msg);
            if (!res){
                element.value = "";
            }
            return false;
        }
    }

    return true;
};

function msgClass()
{
    var self = AbstractClass();
    
    var id          = null; 
    var type        = null;
    var body        = null;
    var time        = null;
 
    var src_chatid  = "0";
    var dst_chatid  = "0";

    self.isIncoming = function(){
        var ret;
        if (type == "incoming"){
            ret = 1;
        } else if (type == "typeing"){
            ret = 1;
        } else if (type == "delivered"){
            ret = 1; 
        } else {
            ret = 0;
        }
        return ret;
    }
    self.setId = function(new_id){
        id = new_id;
    }
    self.getId = function(){
        return id;
    }
    self.getType = function(){
        return type;
    }
    self.getSrcChatId = function(){
        return src_chatid;
    }
    self.getDstChatId = function(){
        return dst_chatid;
    }
    self.setSrcChatId = function(new_chatid){
        if (!new_chatid){
            new_chatid = "0";
        }
        if (! /^\d+$/.test(new_chatid)){
            perror("Invalid chat ID: '"+new_chatid+"'");
            return null;
        }
        src_chatid = new_chatid;
        return src_chatid;
    } 
    self.setDstChatId = function(new_chatid){
        if (!new_chatid){
            new_chatid = "0";
        }
        if (! /^\d+$/.test(new_chatid)){
            perror("Invalid chat ID: '"+new_chatid+"'");
            return null;
        }
        dst_chatid = new_chatid;
        return dst_chatid;
    } 
    self.setType = function(new_type){
        if (   new_type != "incoming"
           && new_type  != "outcoming"
           && new_type  != "delivered"
           && new_type  != "typeing"
           && new_type  != "system")
        {
            perror("Invalid message type: '"+new_type+"'");
            return null;
        }
        type = new_type;
        return type;
    }

    self.getTime = function(){
        return time;
    }

    self.setTime = function(new_time){
        time = new_time;
    }

    self.setBody = function(new_body){
        body = new_body;
    }

    self.getBody = function(){
        return body;
    }

    return self;
};

function msgWindowClass(args)
{
    if (!args.context){
        perror("Missing argument 'args.context'");
        return null;
    }
    if (!args.huid){
        perror("Missing argument 'args.huid'");
        return null;
    }

    // don't close window, hide it
    args["hide_instead_close"] = 1;

    var messages = [];
    var self = AbstractClass("WindowClass", args);

    var context     = args.context; 
    var context_doc = context.document;
    var last_status_line_update = 0;

    self.huid = args.huid;
    self.name = args.name;

    var tmpl = TemplateMessageWindowClass();
    tmpl.onKeyDown("return keyPress(this, event);");
    tmpl.onShowHistory("return showHistory(this, event);");

    self.setSize = function (x, y){
        self.WindowClass.setSize(x, y);
        tmpl.setSize(x - 45, y - 50);
        tmpl.refresh();
    }

    function createHistoryLine(msg_class){
        tr = context_doc.createElement("tr");
        td = context_doc.createElement("td");
        td.appendChild(context_doc.createTextNode(msg_class.getTime()));
        tr.appendChild(td);
        td = context_doc.createElement("td");
        td.setAttribute("width", "100%");
        td.innerHTML = filter(msg_class.getBody());
        tr.appendChild(td);
        return tr;
    };

    function clearStatusLine(){
        var cur_date            = new Date;
        var unixtime_ms         = cur_date.getTime();
        var unixtime            = parseInt(unixtime_ms / 1000);
        if (((unixtime - last_status_line_update) > 3)){
            // todo check before clean
            tmpl.setStatus("");
        }
    }
    setInterval(clearStatusLine, 1000);

    self.delMsg = function(msg_obj){
        for (var i = 0; i < messages.length; i++){
            tmpl.delMesg(msg_obj)
            if (messages[i].getId() == msg_obj.getId()){
                cleanClass(messages[i]);
            }
            cleanClass(msg_obj);
        } 
    }

    self.recvMsg = function(msg_obj){
        if (msg_obj.getType() == "typeing"){
            // it's typing event
            var cur_date            = new Date;
            var unixtime_ms         = cur_date.getTime();
            var unixtime            = parseInt(unixtime_ms / 1000);
            last_status_line_update = unixtime;
            tmpl.setStatus("печатает..");
            return;
        }
        if (msg_obj.getType() == "incoming"){
            context.sound.loadSound("/web_InI/sound/msg.mp3", "true");
            messages.push(msg_obj);
            main_window.startBlink(self.huid);
        }
        tmpl.addMesg(msg_obj);
    }

    self.syncHistory = function(msg_objs){
        for (var i = 0; i < messages.length; i++){
            cleanClass(messages[i]);
        }
        messages = [];
        for (var i = 0; i < msg_objs.length; i++){
            var msg_obj = msg_objs[i];
            messages.push(msg_obj);
            if (i == 0){
                // clear all messages before
                tmpl.clearHistory(msg_obj);
            }
            tmpl.addMesg(msg_obj);
        }
        tmpl.refresh();
    }

    self.sendMsg = function(msg)
    {
        main_window.stopBlink(self.huid); 
        ipnoise_api.API_sendmsg(self.huid, msg);
    };

    self.focus = function()
    {
        self.WindowClass.focus();
        main_window.stopBlink(self.huid);
    };

    self.unhide = function()
    {
        main_window.stopBlink(self.huid);
        self.WindowClass.unhide();
    };
 
    self.showContextMenu = function (ev){
        var node    = ev.target;
        var chat_id = -1; 
        var msg_id  = -1;
        while (node){
            var regexp;
            var res;
            var className = node.className;
            if (msg_id < 0){
                regexp = /msg_id_(\d+)/;
                res = regexp.exec(className);
                if (res){
                    msg_id = res[1];
                }
            } else {
                regexp = /chatbody_srcid_(\d+)/;
                res = regexp.exec(className);
                if (res){
                    chat_id = res[1];
                    break;
                }
            }
            node = node.parentNode;
        };
        if (chat_id < 0 || msg_id <= 0){
            return true;
        }
        var elements = [
            {
                "img":      "",
                "text":     "Удалить из истории",
                "class":    "contextMenuDelMsg",
                "onclick":  "ipnoise_api.API_delmsg({"
                        +" huid:        '"+self.huid+"',"
                        +" chat_id:     '"+chat_id+"',"
                        +" msg_id:     '"+msg_id+"'," 
                        +"});"
            },
        ];
        // context_win must be global
        context_win = contextWindowClass({
            "context":          context,
            "ev":               context.window.event || ev,
            "elements":         elements
        });           
        return false;
    }
   
    if (args["sync_history"]*1){
        // TODO remove hardcode - 0 (chatid)
        ipnoise_api.API_synchistory(self.huid, 0);
    };

    self.setBodyElement(tmpl.main_div);

    self.setSize("600", "400");

    return self;
}

