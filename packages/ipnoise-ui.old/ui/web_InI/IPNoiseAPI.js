/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  IPNoise API class implementation
 *  Aug, 2009.
 */

// API Log window
var ipnoise_api_lwindow     = null;
var IPNOISE_API_EVENTS_URL  = "/API/events/";
var IPNOISE_API_CMD_URL     = "/API/cmd/";

function IPNoiseAPIClass (args)
{
    var context = args.context;
    if (!context){
        perror("Missing argument 'args.context'");
        return null;
    }
    var context_doc = context.document;
    var self = AbstractClass();

    var tmpl_log_win = TemplateLogWindowClass();
    ipnoise_api_lwindow = WindowClass(args);
    ipnoise_api_lwindow.setBodyElement(tmpl_log_win.main_div);
    ipnoise_api_lwindow.setTitle("IPNoise API log window");
    ipnoise_api_lwindow.setSize(600, 500);
    ipnoise_api_lwindow.setAbsPos(5, 50);
    ipnoise_api_lwindow.minimize();

    // ipnoise event socket
    var event_net = null;
    var event_socket_watchdog = null;

    // auth data
    var remote_login    = null;
    var remote_password = null;

    function log(msg_type, msg)
    {
        var cur_date = new Date();
        var year  = cur_date.getFullYear();
        var month = cur_date.getMonth();
        var day   = cur_date.getDate();
        var hour  = cur_date.getHours();
        var min   = cur_date.getMinutes();
        var sec   = cur_date.getSeconds();
        var msec   = cur_date.getMilliseconds();

        year  = text_int_align(year,  4);
        month = text_int_align(month, 2);
        day   = text_int_align(day,   2);
        hour  = text_int_align(hour,  2);
        min   = text_int_align(min,   2);
        sec   = text_int_align(sec,   2);
        msec  = text_int_align(msec,  3);

        var msg_obj = msgClass();
        msg_obj.setType(msg_type);
        msg_obj.setTime(hour+":"+min+":"+sec);
        msg_obj.setBody(msg);

        tmpl_log_win.addMesg(msg_obj);
    }

    function escapeHTML(data){
        var div  = context_doc.createElement('div');
        var text = context_doc.createTextNode(data);
        div.appendChild(text);
        var res = self.XML2Text(div);
        if (res){
            res     = res.replace(/\n/g, "<br>\n");
            res     = res.replace(/\s/g, "&nbsp;");
        }
        return res;
    };

    var answer_part = "";
    function netAnswerReady(data){
        if (data.status != 'SUCCESS'){
            return;       
        }
        var text = data.responseText;
        if (!text.length){
            return;
        }
        answer_part += text;
        var packet = "";
        var packet_start = 0;
        for (var i = 0; i < answer_part.length; i++){
            if (answer_part[i] == "\n"){
                packet = answer_part.substr(packet_start, i);
                packet_start = ++i;
                if (packet.length){
                    try {
                        self.recv(Base64.decode(packet));
                    } catch (e) {
                        perror("Error", e);
                    };
                    packet = "";
                }
            }
        }
        answer_part = answer_part.substr(packet_start);
    }

    function createEventSocket(){
        if (!event_net){
            if (ipnoise_sess_id){
                // we have ipnoise_sess_id and we have not event socket, create            
                event_net               = XmlHttpRequestClass();
                event_net.url           = IPNOISE_API_EVENTS_URL+"?sess_id="+ipnoise_sess_id;
                event_net.onload        = netEventReady;
                event_net.non_block     = 1;
                event_net.sendGet();       
            } else if (remote_login && remote_password){
                // do login
                self.API_login(remote_login, remote_password);
            }
        }
    }

    // start watchdog
    event_socket_watchdog = setInterval(createEventSocket, 2000);

    var ev_answer_part = "";
    function netEventAnswerReady(data){
        if (data.status != 'SUCCESS'){
            return;       
        }
        var text = data.responseText;
        if (!text.length){
            return;
        }
        ev_answer_part += text;
        var packet = "";
        var packet_start = 0;
        for (var i = 0; i < ev_answer_part.length; i++){
            if (ev_answer_part[i] == "\n"){
                packet = ev_answer_part.substr(packet_start, i);
                packet_start = ++i;
                if (packet.length){
                    try {
                        self.recv(Base64.decode(packet));
                    } catch (e) {
                        perror("Error: ", e);
                    };
                    packet = "";
                } 
            }
        }
        ev_answer_part = ev_answer_part.substr(packet_start);
    }

    function netEventReady(data){
        if (data.status == 'DISCONNECT'){
            event_net = null;
            if (data.status_code == 500){
                // auth required
                ipnoise_sess_id = null;
            }
        } else if (data.status == 'SUCCESS'){
            netEventAnswerReady(data);
        }
    }

    // InI API CMD transport
    var net         = XmlHttpRequestClass();
    net.url         = IPNOISE_API_CMD_URL;
    net.onload      = netAnswerReady;

    self.send = function (command){
        log("outcoming", escapeHTML(command));
        net.sendPost(command);
    };

    var items = {};

    self.recv = function (data){
        if (!data){
            return;
        }
        log("incoming", escapeHTML(data));

        var parser = new DOMParser();
        var xmlDoc = parser.parseFromString(data, "text/xml");

        // get sessid
        var ipnoise = xmlDoc.getElementsByTagName("ipnoise")[0];
        if (ipnoise){
            var sess_id = ipnoise.getAttribute("sess_id");
            if (sess_id){
                ipnoise_sess_id = sess_id;
            }
        }

        var events = xmlDoc.getElementsByTagName("events")[0];
        if (!events){
            return;
        }
        var events = events.getElementsByTagName("event");
        for (var i = 0; i < events.length; i++){
            var event = events[i];
            var event_type = event.getAttribute("type");

            if (/^update-contact-list$/i.test(event_type)){
                // update-contact-list event
                var items1 = event.getElementsByTagName("items")[0];
                if (items1){
                    items1 = items1.getElementsByTagName("item");
                }
                items = {};
                for (var i = 0; i < items1.length; i++){
                    var item        = items1[i];
                    var item_id     = item.getAttribute("huid");
                    var name        = item.getAttribute("name");
                    items[item_id]  = {
                        name:  name 
                    };
                }
                // call handler
                main_window.setContactList(event);
            } else if (/^whoaround$/i.test(event_type)){ 
                var items_els = event.getElementsByTagName("item");
                var items_arr = [];
                for (var i = 0; i < items_els.length; i++){
                    var item_el  = items_els[i];

                    var huid  = item_el.getAttribute("huid");
                    var name  = item_el.getAttribute("name");
                    var descr = item_el.getAttribute("descr");
                    var status_icon = item_el.getAttribute(
                        "status_icon");
                    var avatar_icon = item_el.getAttribute(
                        "avatar_icon");
                    
                    var item_obj = itemClass();
                    item_obj.setHuid(huid);
                    item_obj.setName(name);
                    item_obj.setDescr(descr);
                    item_obj.setStatusIcon(status_icon);
                    item_obj.setAvatarIcon(avatar_icon);
                    items_arr.push(item_obj);
                };

                if (items_arr.length){
                    var classes  = getAllClasses();
                    for (var class_id in classes){
                        // search already opened msg windows
                        // and update history
                        var class_obj = classes[class_id];
                        if (class_obj
                            && class_obj.abstract.class_name == "whoAroundWindowClass")
                        {
                            class_obj.setResults(items_arr);
                        } 
                    }
                }
            } else if (/^del-msg$/i.test(event_type)){
                var huid        = event.getAttribute("huid"); 
                var msg         = event.getElementsByTagName("msg")[0];
                var msg_id      = msg.getAttribute("id");  
                var src_chatid  = msg.getAttribute("src_chatid");

                var msg_obj = msgClass();
                msg_obj.setId(msg_id); 
                msg_obj.setSrcChatId(src_chatid);

                var classes = getAllClasses();
                for (var class_id in classes){
                    // search already opened msg windows
                    var class_obj = classes[class_id];
                    if (class_obj
                        && class_obj.abstract.class_name == "msgWindowClass"
                        && class_obj.huid == huid)
                    {
                        class_obj.delMsg(msg_obj);
                    } 
                }
            } else if (/^search-history$/i.test(event_type)){
                var huid           = event.getAttribute("huid"); 
                var chats_element  = event.getElementsByTagName("chats")[0];
                var chats_elements = chats_element.getElementsByTagName("chat");

                var chats = [];

                for (var i = 0; i < chats_elements.length; i++){
                    var chat_element = chats_elements[i];
                    var messages     = chat_element.getElementsByTagName("msg");
                    var chat = {
                        id:             chat_element.getAttribute("id"),
                        create_date:    chat_element.getAttribute("create_date"),
                        msg_count:      chat_element.getAttribute("msg_count"),
                        messages:       []
                    };
                    for (var m = 0; m < messages.length; m++){
                        var msg         = messages[m];
                        var msg_id      = msg.getAttribute("id"); 
                        var msg_type    = msg.getAttribute("msg_type");
                        var time        = msg.getAttribute("time");
                        var src_chatid  = msg.getAttribute("src_chatid");
                        var dst_chatid  = msg.getAttribute("dst_chatid");
                        var compared    = msg.getAttribute("compared");
 
                        if (msg && msg.hasChildNodes()){
                            msg = Base64.decode(msg.firstChild.nodeValue);
                        } else {
                            msg = "";
                        }

                        var msg_obj = msgClass();
                        msg_obj.setId(msg_id); 
                        msg_obj.setType(msg_type);
                        msg_obj.setTime(time);
                        msg_obj.setBody(msg);
                        msg_obj.setSrcChatId(src_chatid);
                        msg_obj.setDstChatId(dst_chatid);

                        if (compared){
                            msg_obj.setType("system");
                        }
 
                        chat.messages.push(msg_obj);
                    }
                    chats.push(chat);
                }
               
                var classes  = getAllClasses();
                for (var class_id in classes){
                    // search already opened msg windows
                    // and update history
                    var class_obj = classes[class_id];
                    if (class_obj
                        && class_obj.abstract.class_name == "historyWindowClass"
                        && class_obj.huid == huid)
                    {
                        class_obj.setHistory(chats);
                    } 
                }
            } else if (/^update-history$/i.test(event_type)){
                var huid        = event.getAttribute("huid");
                var msg         = event.getElementsByTagName("msg")[0];
                var msg_id      = msg.getAttribute("id");  
                var msg_type    = msg.getAttribute("msg_type");
                var time        = msg.getAttribute("time");
                var src_chatid  = msg.getAttribute("src_chatid");
                var dst_chatid  = msg.getAttribute("dst_chatid");
                if (msg && msg.hasChildNodes()){
                    msg = Base64.decode(msg.firstChild.nodeValue);
                } else {
                    msg = "";
                }

                var msg_obj = msgClass();
                msg_obj.setId(msg_id); 
                msg_obj.setType(msg_type);
                msg_obj.setTime(time);
                msg_obj.setBody(msg);
                msg_obj.setSrcChatId(src_chatid);
                msg_obj.setDstChatId(dst_chatid);

                // search already opened msg windows
                // or create new
                var win = createMsgWindow({
                    "huid":         huid,
                    "name":         items[huid] ? items[huid].name : "UNKNOWN",
                    "sync_history": 0,
                    "show":         0
                });

                win.recvMsg(msg_obj);

            } else if (/^sync-history$/i.test(event_type)){
                var huid = event.getAttribute("huid");
                var msg_elements = event.getElementsByTagName("msg");
                var msgs = [];

                for (var i = 0; i < msg_elements.length; i++){
                    var element = msg_elements[i];
                    var msg = "";
                    if (element && element.firstChild){
                        msg = element.firstChild.nodeValue;
                    }
                    var msg_id      = element.getAttribute("id");
                    var msg_type    = element.getAttribute("msg_type");
                    var time        = element.getAttribute("time");
                    var src_chatid  = element.getAttribute("src_chatid");
                    var dst_chatid  = element.getAttribute("dst_chatid");
 
                    var msg_obj = msgClass();
                    msg_obj.setId(msg_id);
                    msg_obj.setType(msg_type);
                    msg_obj.setTime(time);
                    msg_obj.setBody(Base64.decode(msg));
                    msg_obj.setSrcChatId(src_chatid);
                    msg_obj.setDstChatId(dst_chatid);
                    msgs.push(msg_obj);
                }
                var classes = getAllClasses();
                for (var class_id in classes){
                    // search already opened msg windows
                    // and update history
                    var class_obj = classes[class_id];
                    if (class_obj
                        && class_obj.abstract.class_name == "msgWindowClass"
                        && class_obj.huid == huid)
                    {
                        class_obj.syncHistory(msgs);
                    } 
                }
            }
        }
    };
    self.API_delmsg  = function(args){
        var huid    = args["huid"]; 
        var chat_id = args["chat_id"];
        var msg_id  = args["msg_id"];
        var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='del-msg'"
            +"               huid='"+huid+"'"
            +"               chat_id='"+chat_id+"'"
            +"               msg_id='"+msg_id+"'"
            +"      />\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);

    }
    self.API_additem = function(args){
        var huid     = args["huid"];
        var group_id = args["group_id"]; 
        var name     = args["name"];
        var descr    = args["descr"];
        if (!group_id){
            group_id = "";
        }
        if (!huid){
            huid = "";
        }
        if (name){
            name = Base64.encode(name);
        } else {
            name = "";
        }
        if (descr){
            descr = Base64.encode(descr);
        } else {
            descr = "";
        }
        var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='add-item'"
            +"               huid='"+huid+"'" 
            +"               group_id='"+group_id+"'"
            +"               name='"+name+"'"
            +"               descr='"+descr+"'"
            +"      />\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);
    }
    self.API_delitem = function(args){
        var huid     = args["huid"]; 
        var group_id = args["group_id"];
        if (!huid
            || huid == null
            || typeof huid == 'undefined')
        {
            perror("Empty huid", args);
            return;
        }
        var cmd  = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n";
            cmd += "<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n";
            cmd += "  <commands>\n";
            cmd += "      <command type='del-item'";
            cmd += "          huid='"+huid+"'";
            if (/^\d+$/.test(group_id)){
                cmd += "      group_id='"+group_id+"'";
            }
            cmd += "      />\n";
            cmd += " </commands>\n";
            cmd += "</ipnoise>\n";
        self.send(cmd);
    }
    self.API_gethistory = function(huid, src_chatid){
        var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='get-history' huid='"+huid+"' src_chatid='"+src_chatid+"'/>\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);
    }

    self.API_synchistory = function(huid, src_chatid){
        var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='sync-history' huid='"+huid+"' src_chatid='"+src_chatid+"'/>\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);
    }

    self.API_searchhistory = function(args){
        var chatіd = args.chatid;
        var huid   = args["huid"];
        var msg    = args["msg"];

        var cmd  = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n";
            cmd += "<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n";
            cmd += "  <commands>\n";
            cmd += "      <command type='search-history' ";
            cmd += " huid='"+huid+"'";
            if (typeof chatіd != 'undefined'){
                cmd += " chatid='"+chatіd+"'";
            }
            if (typeof msg != 'undefined'){
                cmd += " msg='"+msg+"'";
            }
            cmd += "/>\n";
            cmd += " </commands>\n";
            cmd += "</ipnoise>\n";
        self.send(cmd);
    }

    self.API_sendmsg = function(dsthuid, msg){
        var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='sendmsg' dsthuid='"+dsthuid+"'"
                                         +" src_chatid='"+msg.getSrcChatId()+"'"
                                         +" dst_chatid='"+msg.getDstChatId()+"'>\n"
            +"          <msg><![CDATA["+Base64.encode(msg.getBody())+"]]></msg>\n"
            +"      </command>\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);
    }

    self.API_whoaround = function(){
         var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='whoaround'/>\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);
    };

    self.API_login = function(login, password){
        remote_login    = login
        remote_password = password;
        var cmd = '<?xml version="1.0" encoding="UTF-8" ?>'+"\n"
            +"<ipnoise sess_id=\""+ipnoise_sess_id+"\">\n"
            +"  <commands>\n"
            +"      <command type='login'>\n"
            +"          <login><![CDATA["   +login+   "]]></login>\n"
            +"          <password><![CDATA["+password+"]]></password>\n"
            +"      </command>\n"
            +" </commands>\n"
            +"</ipnoise>\n";
        self.send(cmd);
    };

    return self;
}

