// vim:syntax=javascript tabstop=4 expandtab

$.fn.callWidgetMethod = function(
    a_method,
    a_args)
{
    var self    = this;
    var ret     = undefined;

//    var class_name  = arguments[0];
//    var obj         = arguments[1];
//    var args = Array.prototype.slice.call(arguments, 1);

    // loop though the data and check each piece of data to
    // see if it has the method
    var data = self.data();
    for (var cur_key in data){
        var res     = undefined;
        var key     = undefined;
        var cur_val = data[cur_key];

        if (!$.isFunction(cur_val[a_method])){
            continue;
        }

        res = /ipnoise(.+)/.exec(cur_key);
        if (res){
            // I don't known why, but jquery will add
            // namespace as prefix to class name
            // example: ipnoiseIPNoiseObjectCamera
            // so do remove 'ipnoise' prefix
            key = res[1];
        }

        getLogger().pdebug(100, "before callWidgetMethod:\n"
            +"  cur_key:        '"+cur_key+"'\n"
            +"  cur_val:        '"+cur_val+"'\n"
            +"  key:            '"+key+"'\n"
            +"  method:         '"+a_method+"'\n",
            a_args
        );


        // a_val[a_method].apply(self, args);
        //ret = self[key](a_method, a_args);
        ret = cur_val[a_method](a_args);
        getLogger().pdebug(100, "after callWidgetMethod:\n" 
            +"  ret: '"+ret+"'\n"
        );
        
        break;
        //self[key].apply(self, args);
    }

    return ret;
}

var SESSION_ID                          = undefined;
var queue_api_packets                   = {};
var send_api_packets                    = {};
var send_api_packets_timer              = undefined;
var send_api_packets_last_send_time_ms  = 0;
var next_object_id                      = 1;
var requests_cache_time_ms              = 100;

function getSessId(){
    var sessid = SESSION_ID;
    //if (!sessid){
    //    sessid     = readCookie("sessid");
    //    SESSION_ID = sessid;
    //}
    return sessid;
}

function setSessId(a_sessid){
    SESSION_ID = a_sessid;
}

function flushApiPackets(a_req_id){
    var packets = [];

    for (var req_id in queue_api_packets){
        var packet = queue_api_packets[req_id];

        if (    undefined != a_req_id
            &&  req_id    != a_req_id)
        {
            // if req id provided, send only him
            continue;
        }

        if (send_api_packets[req_id]){
            // skip if already sent
            continue;
        }
        send_api_packets[req_id] = 1;
        packets.push(packet);
    }

    var data = {
        'requests': packets
    };

    if (packets.length){
        send_api_packets_last_send_time_ms = new Date().getTime();
        var send_api_args = {
            'data':     data,
            'success':  function(a_data){
                getLogger().pdebug(
                    30,
                    "after sendApi",
                    a_data
                );
                if ($.isEmptyObject(a_data)){
                    getLogger().pfatal(
                        "empty answer, packets:",
                        packets
                    );
                }
                for (var req_id in a_data){
                    var packet = undefined;
                    var answer = a_data[req_id];
                    packet = queue_api_packets[req_id];
                    delete queue_api_packets[req_id];
                    delete send_api_packets[req_id];
                    if (packet){
                        if (packet.success){
                            packet.success(answer);
                        }
                    } else {
                        getLogger().perror("cannot get packet"
                            +" for req_id: '"+req_id+"'"
                        );
                    }
                }
                // create new timer
                //if (send_api_packets_timer){
                //    clearInterval(send_api_packets_timer);
                //    send_api_packets_timer = undefined;
                //}
                //send_api_packets_timer = setTimeout(
                //    flushApiPackets,
                //    100
                //);
            },
            'failed':   function(a_data){
                for (var i = 0; i < packets.length; i++){
                    var packet = packets[i];
                    var req_id = packet.req_id;
                    delete queue_api_packets[req_id];
                    delete send_api_packets[req_id];
                    if (packet.failed){
                        packet.failed();
                    }
                }
                //if (send_api_packets_timer){
                //    clearInterval(send_api_packets_timer);
                //    send_api_packets_timer = undefined;
                //}
                // create new timer
                //send_api_packets_timer = setTimeout(
                //    flushApiPackets,
                //    100
                //);
            }
        };
        sendApi(send_api_args);
    }
}

function getNextPacketRequestId(){
    var req_id = next_object_id++;
    //new Date().getTime() + '';
    return req_id;
}

function sendApiPackets(a_args){
    var args = {
        'packets':  undefined,
        'flush':    undefined,
        'single':   undefined
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var cur_time_ms = new Date().getTime();
    var packets     = args.packets;
    var flush       = args.flush;
    var single      = args.single;

    if (undefined == packets){
        getLogger().pfatal("missing argument: 'packets'",
            args
        );
    }

    // clear timer if exist
    if (send_api_packets_timer){
        clearInterval(send_api_packets_timer);
        send_api_packets_timer = undefined;
    }

    // process packets
    for (var i = 0; i < packets.length; i++){
        var packet = packets[i];
        var req_id = getNextPacketRequestId();
        packet.req_id = req_id;
        // TODO use 'namespaces' instead webgl=1
        // example: 'namespace': 'webgl'
        // 20140806 morik@
        packet.webgl    = 1;
        packet.sessid   = getSessId();
        if (packet.prepare){
            packet.prepare();
        }

        // queue packet
        queue_api_packets[req_id] = packet;

        if (single){
            flushApiPackets(req_id);
            break;
        }
    }

    var delta_time_ms = cur_time_ms
        - send_api_packets_last_send_time_ms;

    if (    flush
        ||  (requests_cache_time_ms <= delta_time_ms))
    {
        flushApiPackets();
    } else {
        // create new timer
        send_api_packets_timer = setTimeout(
            flushApiPackets,
            requests_cache_time_ms
        );
    }
}

function sendApiPacket(a_args){
    var args = {
        'packet':   undefined,
        'flush':    undefined,
        'single':   undefined
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var packet  = args.packet;
    var flush   = args.flush;
    var single  = args.single;

    if (undefined == packet){
        getLogger().pfatal("missing argument: 'packet'",
            args
        );
    }

    var data = {
        'packets':  [ packet ],
        'flush':    flush,
        'single':   single
    };

    sendApiPackets(data);
}

function sendApi(a_args){
    var headers = {
        'Content-Type': 'application/json; charset=utf-8'
    };
    var url     = '/api';
    var data    = JSON.stringify(a_args.data);

    $.ajax({
        'type':           'POST',
        'url':            url,
        'data':           data,
        'dataType':       'json',
        'headers':        headers,
        'success':        function (a_data, a_text_status){
            if (a_args.success){
                // all ok
                a_args.success(a_data);
            }
        },
        'error': function (a_err){
            // network error
            getLogger().perror(
                'network error: '+JSON.stringify(a_err)
            );
            if (a_args.failed){
                a_args.failed(a_err);
            }
        }
    });
}

function parseHtmlOnWidgets(a_args)
{
    var args = {
        'html':         undefined,
        'html_base64':  undefined,
        'el':           $('body'),
        'args':         {},
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var html        = args.html;
    var html_base64 = args.html_base64;
    if (undefined != html_base64){
        html = Base64.decode(html_base64);
    }
    if (undefined == html){
        getLogger().pfatal(
            "at least one argument must be defined:"
            +" 'html' or 'html_base64'"
        );
    }

    var nodes   = $.parseHTML(html);
    var widgets = [];
    var el      = args.el;

    for (var i = 0; i < nodes.length; i++){
        var node = nodes[i];
        if ("DIV" == node.tagName){
            var div           = $(node);
            var found_widgets = initWidgets({
                'el':                   div,
                'args':                 args.args,
                'fatal_if_not_found':   1
            });
            widgets = widgets.concat(found_widgets);
        }
    }

    el.append(widgets);

    return widgets;
}

function render(a_args){
    var args = {
        'method':       'render',
        'object_id':    undefined,
        'el':           $('body'),
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'webgl':        0,
        'params':       {
            'view': 'widget'
        },
        'args':         {}
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    if (    !args.args
        ||  !args.args.viewer)
    {
        getLogger().pfatal("cannot render without viewer");
    }

    var packet = {
        'object_id':    args.object_id,
        'method':       args.method,
        'params':       args.params,
        'prepare':      args.prepare,
        'success':      function(a_data){
            var params = a_data.params;
            if (params && params.html){
                var widgets = parseHtmlOnWidgets({
                    'el':           args.el,
                    'html_base64':  params.html,
                    'args':         args.args
                });
                if (args.success){
                    args.success(a_data, widgets);
                }
            } else {
                if (args.failed){
                    args.failed(a_data);
                }
            }
        },
        'failed':       function(a_data){
            if (args.failed){
                args.failed(a_data);
            }
        }

    };

    if (args.webgl){
        packet.params.webgl = args.webgl;
    }

    var data = {
        'packet':   packet
    };
    sendApiPacket(data);
}

function renderEmbedded(a_args){
    var args = {
        'method':   'renderEmbedded'
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    if (    !args.args
        ||  !args.args.viewer)
    {
        getLogger().pfatal(
            "cannot renderEmbedded without viewer"
        );
    }

    render(args);
}

function gotoObject(a_redirect){
    var object_id = a_redirect.object_id;
    var view      = a_redirect.view;
    var url       = '/api?object_id='+object_id;
    if (view){
        url += '&view='+view;
    }
    $(location).attr('href', url);
}

function callMethod(a_args){
    var args = {
        'method':       undefined,
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {}
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var object_id = args.object_id;
    var method    = args.method;

    if (undefined == object_id){
        getLogger().pfatal("missing argument: 'object_id'",
            args
        );
    }

    if (undefined == method){
        getLogger().pfatal("missing argument: 'method'",
            args
        );
    }

    var packet = {
        'object_id':    object_id,
        'method':       method,
        'params':       args.params,
        'prepare':      args.prepare,
        'success':      function(a_data){
            var params = a_data.params;
            //if (params && params.redirect){
            //    gotoObject(params.redirect);
            //}
            if (a_args.success){
                a_args.success(a_data);
            }
        },
        'failed':       function(a_data){
            var params = a_data.params;
            //if (params && params.redirect){
            //    gotoObject(params.redirect);
            //}
            if (a_args.failed){
                a_args.failed(a_data);
            }
        }
    };
    var data = {
        'packet':   packet,
    };
    sendApiPacket(data);
}

function updateObject(a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       undefined
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'updateObject';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    callMethod(args);
}

function addToBookmarks(a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {}
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'addToBookmarks';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    callMethod(args);
}

function createObject(a_object_id, a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {}
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'createObject';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    callMethod(args);
}

function NeighUnlink(a_object_id, a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {
            'neigh_id':     undefined
        }
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'neighUnlink';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    var neigh_id = params.neigh_id;
    if (undefined == neigh_id){
        getLogger().pfatal(
            "missing argument: 'params.neigh_id'",
            args
        );
    }

    callMethod(args);
}

function NeighDel(a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {
            'neigh_id':     undefined
        }
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'neighDel';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    var neigh_id = params.neigh_id;
    if (undefined == neigh_id){
        getLogger().pfatal(
            "missing argument: 'params.neigh_id'",
            args
        );
    }

    callMethod(args);
}

function NeighAdd(a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {
            'neigh_id':     undefined
        }
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'neighAdd';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    var neigh_id = params.neigh_id;
    if (undefined == neigh_id){
        getLogger().pfatal(
            "missing argument: 'params.neigh_id'",
            args
        );
    }

    callMethod(args);
}

function BookmarkAdd(a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {
            'neigh_id':     undefined
        }
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'bookmarkAdd';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    var neigh_id = params.neigh_id;
    if (undefined == neigh_id){
        getLogger().pfatal(
            "missing argument: 'params.neigh_id'",
            args
        );
    }

    callMethod(args);
}

function BookmarkDel(a_args){
    var args = {
        'object_id':    undefined,
        'prepare':      undefined,
        'success':      undefined,
        'failed':       undefined,
        'params':       {
            'neigh_id':     undefined
        }
    }

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    // setup method name
    args.method = 'bookmarkDel';

    var object_id = args.object_id;
    var params    = args.params;

    if (undefined == object_id){
        getLogger().pfatal(
            "missing argument: 'object_id'",
            args
        );
    }

    if (undefined == params){
        getLogger().pfatal(
            "missing argument: 'params'",
            args
        );
    }

    var neigh_id = params.neigh_id;
    if (undefined == neigh_id){
        getLogger().pfatal(
            "missing argument: 'params.neigh_id'",
            args
        );
    }

    callMethod(args);
}

// neighs operations
function processNeigh(a_action, a_ev){
    var res;
    var tr              = $(a_ev.target).parents('.neigh');
    var class_name      = $(tr).attr('class');
    var classes         = class_name.split(' ');
    var object_id       = '';
    var object_type     = 0;
    var object_type_str = '';

    for (var i = 0; i < classes.length; i++){
        var cur_class = classes[i];
        res = /object_id_(.+)/.exec(cur_class);
        if (res){
            object_id = res[1];
            continue;
        }
        res = /object_type_(\d+)/.exec(cur_class);
        if (res){
            object_type = res[1];
            continue;
        }
        res = /object_type_str_(\D+)/.exec(cur_class);
        if (res){
            object_type_str = res[1];
            continue;
        }
    }

    var res = confirm(
        "Are you sure to "+a_action+" neigh?\n"
        +"Neigh ID:   '"+object_id+"'\n"
        +"Neigh Type: "+object_type+" ("+object_type_str+")\n",
        0
    );
    if (!res){
        return;
    }

    if ('unlink' == a_action){
        NeighUnlink(
            getCurObjectId(),
            {
                'neigh_id': object_id
            }
        );
    } else if ('delete' == a_action){
        NeighDel(
            getCurObjectId(),
            {
                'neigh_id': object_id
            }
        );
    } else if ('add' == a_action){
        NeighAdd(
            getCurObjectId(),
            {
                'neigh_id': object_id
            }
        );
    } else if ('bookmark_add' == a_action){
        BookmarkAdd(
            getCurObjectId(),
            {
                'neigh_id': object_id
            }
        );
    } else if ('bookmark_del' == a_action){
        BookmarkDel(
            getCurObjectId(),
            {
                'neigh_id': object_id
            }
        );
    } else {
        getLogger().perror('invalid action: "'+a_action+'"');
    }
}

function createCookie(name, value, days){
    var expires;

    if (days){
        var date = new Date();
        date.setTime(
            date.getTime() + (
                days * 24 * 60 * 60 * 1000
            )
        );
        expires = "; expires=" + date.toGMTString();
    } else {
        expires = "";
    }
    document.cookie = escape(name)
        + "="
        + escape(value)
        + expires
        + "; path=/";
}

function readCookie(name){
    var nameEQ  = escape(name) + "=";
    var ca      = document.cookie.split(';');
    for (var i = 0; i < ca.length; i++){
        var c = ca[i];
        while (c.charAt(0) === ' '){
            c = c.substring(1, c.length);
        }
        if (c.indexOf(nameEQ) === 0){
            return unescape(c.substring(nameEQ.length, c.length));
        }
    }
    return undefined;
}

function eraseCookie(name){
    createCookie(name, "", -1);
}

var events_subscr       = {};
var events_subscr_timer = undefined;
function apiUpdateEventsSubscr(a_args){
    var args = {
        'object_id':    undefined,
        'events':       undefined
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var object_id   = args.object_id;
    var events      = args.events;

    if (undefined == object_id){
        getLogger().pfatal("missing argument: 'object_id'",
            args
        );
    }
    if (undefined == events){
        getLogger().pfatal("missing argument: 'events'",
            args
        );
    }

    var packet = {
        'method':       'updateEventsSubscr',
        'object_id':    object_id,    
        'params':       {
            'events':   events
        }
    };

    // send request
    var data = {
        'packet':  packet
    };
    sendApiPacket(data);
}

function createWidget(
    a_args)
{
    var args = {
        'el':           $('<div>'),
        'append_to':    $('body'),
        'object_id':    undefined,
        'class_name':   undefined,
        'success':      undefined,
        'failed':       undefined,
        'args':         {},
    };

    // copy args
    copy_args(args, a_args);

    if (!args.args.viewer){
        getLogger().pfatal(
            "cannot create widget without viewer"
        );
    }

    var el          = args.el;
    var class_name  = args.class_name;
    var append_to   = args.append_to;
    var object_id   = args.object_id;

    if (class_name){
        el.addClass(class_name);
        initWidget(args);
        el.appendTo(append_to);
        if (args.success){
            args.success({}, [ el ]);
        }
    } else if (object_id){
        renderEmbedded({
            'object_id':    object_id,
            'args':         args.args,
            'success':      function(a_data, a_widgets){
                if (args.success){
                    args.success(a_data, a_widgets);
                }
            },
            'failed':       function(a_data){
                if (args.failed){
                    args.failed(a_data);
                }
            }
        });
    } else {
        getLogger().pfatal(
            "missing args 'class_name', 'object_id'"
            +" at least one must be defined",
            args
        );
    }
}

function initWidget(
    a_args)
{
    // TODO XXX
    // some times we need pass args to constructor,
    // (not via 'options') because it is not 'options'
    // workaround:
    // do create widget in 2 step:
    // 1. call constructor and create object
    // 2. call object method new(args)
    // For this case was create this function
    // 20140731 morik@

    var widget      = undefined;
    var el          = a_args.el;
    var class_name  = a_args.class_name;
    var args        = a_args.args;

    getLogger().pdebug(50, "trying to init widget,"
        +" class name: '"+class_name+"'"
    );

    //    if (    !args.viewer
    //        &&  ('IPNoiseWebglViewer' != class_name))
    //    {
    //        getLogger().pfatal(
    //            "cannot create class: '"+class_name+"'"
    //            +" missing argument 'viewer'"
    //        );
    //    }

    if (!el){
        getLogger().pfatal("missing argument 'el'");
    }
    if (!class_name){
        class_name = getIPNoiseClassName(el);
    }
    if (!class_name){
        getLogger().pfatal("missing class name\n");
    }

    if (!el[class_name]){
        getLogger().pfatal(
            "cannot create new class: '"+class_name+"',"
            +" class not found",
            el
        );
    }

    el[class_name]();
    el.callWidgetMethod('new', args);

    return el;
}

function initWidgets(a_args){
    var args = {
        'el':                   undefined,
        'fatal_if_not_found':   0,
        'args':                 {},
    };
    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var el                  = $(args.el);
    var viewer              = $(args.viewer);
    var fatal_if_not_found  = args.fatal_if_not_found;
    var widgets_found       = [];

    //if ('object' != JSON.parse(viewer)){
    //    getLogger().pfatal("missing argument 'viewer'"
    //        +" or it is not object"
    //    );
    //}

    if ('object' != jQuery.type(el)){
        getLogger().pfatal("missing argument 'el'"
            +" or it is not object"
        );
    }

    for (var class_name in $.ipnoise){
        var res;
        var els = [];
        if (el.hasClass(class_name)){
            els.push(el);
        }
        res = el.find('.'+class_name);

        getLogger().pdebug(50,
            "initWidgets, was found: '"+res.length+"'"
            +" widgets for class: '"+class_name+"'"
        );

        if (res.length){
            els = els.concat(res);
        }

        for (var i = 0; i < els.length; i++){
            var cur_el = $(els[i]);
            var el = initWidget({
                'class_name':   class_name,
                'el':           cur_el,
                'args':         args.args,
            });
            widgets_found.push(el);
        }
    }

    if (    fatal_if_not_found
        &&  !widgets_found.length)
    {
        var html = el[0].outerHTML;
        getLogger().pfatal(
            "no one class found"
            +" for: '"+html+"'"
        );
    }

    return widgets_found;
}

function openSession(a_args){
    var args = {
        'prepare':  undefined,
        'success':  undefined,
        'failed':   undefined,
        'params':   {}
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var params = args.params;
    var sessid = readCookie("sessid");
    if (undefined != sessid){
        params.sessid = sessid;
    }

    var packet = {
        'object_id':    args.object_id,
        'method':       'openSession',
        'params':       args.params,
        'prepare':      args.prepare,
        'success':      function(a_data){
            var sessid = a_data.sessid;
            if (undefined != sessid){
                setSessId(sessid);
                if (args.success){
                    args.success(a_data);
                }
            } else {
                if (args.failed){
                    args.failed(a_data);
                }
            }
        },
        'failed':       function(a_data){
            if (args.failed){
                args.failed(a_data);
            }
        }
    };

    var data = {
        'packet':   packet,
        'single':   1
    };
    sendApiPacket(data);
}

$(function(){
    var ws = new WebSocket("ws://127.0.0.1:2210/api");
 
    ws.onopen = function(){
        console.log("connection opened\n");

        if (0){
            var msg;
            msg = {
                type: "message 1",
                text: "hello world 1",
                date: Date.now()
            };
            ws.send(JSON.stringify(msg));
        }
        
        //ws.send("message_1");

        for (var m = 0; m < 9; m++){
            var msg = "";
            for (var i = 0; i < 5000; i++){
                msg += m;
            }
            ws.send(msg);
        }
    }
 
    ws.onclose = function(){
        console.log("connection closed\n");
    }
 
    ws.onmessage = function(ev){
        console.log("new msg ev.data: "+ev.data);
    }

    ws.onerror = function(ev){
        console.log("connection error ev.data: "+ev.data);
    }


/*
    openSession({
        'success':  function(a_args){
            // init all widgets
            initWidgets({
                'el':       document,
                'args':     {}
            });

            // load events
            getEvents();
        },
        'failed':   function(){
            getLogger().pfatal("cannot open session");
        }
    });
*/
});

