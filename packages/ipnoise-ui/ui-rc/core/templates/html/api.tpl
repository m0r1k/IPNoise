<script>
    var SESSION_ID = null;

    var g_widgets        = {};
    var g_next_widget_id = 0;

    function initWidget(a_info){
        // get script's parent element
        var script_el = document.scripts[
            document.scripts.length - 1
        ];

        var info       = {};
        info.neuron_id = a_info.id;
        info.id        = a_info.id + "_" + g_next_widget_id++;
        info.class     = a_info.class;
        info.div       = script_el.parentNode;

        $(info.div).attr("id", info.id);
        if ('Neuron' != info.class){
            $(info.div).addClass('Neuron');
        }
        $(info.div).addClass(info.class);

        // store info
        g_widgets[info.id] = info;
    }

    function initWidgets(a_el){
        for (var class_name in $.neurons){
            $(a_el).find('.'+class_name)[class_name]();
        }
    }

    $(document).ready(function(){
        // init all widgets
        initWidgets(document);

        // open session
        // setupSession();

//        var test = $.cookie('sessid');
//        alert(test);

//        for (var i = 0; i < 1000; i++){
//            $.removeCookie('morik'+i);
//        }
//        for (var i = 0; i < 200; i++){
//            $.cookie(
//                'morik'+i,
//                "sessid=c4e6c119e54404dfcc28191b220154b3ae8e49e9"
//            );
//        }
//
//        loadContent();
    });

    function loadContent(){
        var load_els = $('.load');

        for (var i = 0; i < load_els.length; i++){
            var load_el     = $(load_els[i]);
            var load_tag    = load_el.get(0).nodeName;
            var parent_el   = load_el.parents();

            if ('IMG' == load_tag){
                var url = load_el.attr('load_url');
                var el  = $('<img/>');
                el.attr('src', url);
                load_el.replaceWith(el);
            } else {
                perror("unsupported tag: '"+load_tag+"'");
            }
        }
    }

    function setupSession(){
        var packet = {
            'method':       'getCreateSession',
            'object_id':    '0'
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
            },
            'failed':   function(data){
                perror('Cannot open session');
            }
        };
        sendApiPacket(args);
    }

    function sendApiPackets(a_args){
        var ret     = [];
        var data    = {
            'requests':   a_args.packets
        };
        for (var i = 0; i < data.requests.length; i++){
            var req_id = new Date().getTime() + '';
            ret.push(req_id);
            data.requests[i]['req_id'] = req_id + '';
        }

        var args = {
            'data':     data,
            'success':  function(data){
                if (a_args.success){
                    a_args.success(data);
                }
            },
            'failed':   function(data){
                if (a_args.failed){
                    a_args.failed(data);
                }
            }
        };
        sendApi(args);

        return ret;
    }

    function sendApiPacket(a_args){
        var ret     = 0;
        var req_ids = [];

        var args    = {
            'packets':  [ a_args.packet ],
            'success':  function(data){
                if (a_args.success){
                    a_args.success(data);
                }
            },
            'failed':   function(data){
                if (a_args.failed){
                    a_args.failed(data);
                }
            }
        };

        req_ids = sendApiPackets(args);
        ret     = req_ids[0];

        return ret;
    }

    function sendApi(a_args){
        var headers = {
//            'Cookie':       SESSION_ID || document.cookie,
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
            'success':        function (data, textStatus){
                if ('success' == data.status){
                    if (a_args.success){
                        // all ok
                        a_args.success(data);
                    }
                } else {
                    if (a_args.failed){
                        // network ok, but api error
                        a_args.failed(data);
                    }
                }
            },
            'error': function (a_err){
                // network error
                perror('network error: '+JSON.stringify(a_err));
                if (a_args.failed){
                    a_args.failed(a_err);
                }
            }
        });
    }

    function render(a_args){
        var selector = 'body';
        if (a_args.selector){
            selector = a_args.selector;
        }
        var el = a_args.el;
        if (!el){
            el = $(selector);
        }

        var packet = {
            'object_id':    a_args.object_id,
            'method':       'renderEmbedded',
            'view':         'widget'
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.html){
                    var html = Base64.decode(params.html);
                    el.append(html);
                    initWidgets(el);
                }
            }
        };
        sendApiPacket(args);
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

    function callMethod(a_method, a_object_id, a_args){
        var packet = {
            'object_id':  a_object_id,
            'method':     a_method,
            'params':     a_args
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
                if (a_args.success){
                    a_args.success(data);
                }
            },
            'failed':   function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
                if (a_args.failed){
                    a_args.failed(data);
                }
            }
        };
        sendApiPacket(args);
    }

    function updateObject(a_object_id, a_args){
        var packet = {
            'object_id':  a_object_id,
            'method':     'updateObject',
            'params':     a_args
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    // MORIK gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function addToBookmarks(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'addToBookmarks',
            'params':       a_args
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function createObject(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'createObject',
            'params':       {
                'type':     a_args.type
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function NeighUnlink(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'neighUnlink',
            'params':       {
                'neigh_id': a_args.neigh_id,
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function NeighDel(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'neighDel',
            'params':       {
                'neigh_id': a_args.neigh_id,
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function NeighAdd(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'neighAdd',
            'params':       {
                'neigh_id': a_args.neigh_id,
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function BookmarkAdd(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'bookmarkAdd',
            'params':       {
                'neigh_id': a_args.neigh_id,
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
    }

    function BookmarkDel(a_object_id, a_args){
        var packet = {
            'object_id':    a_object_id,
            'method':       'bookmarkDel',
            'params':       {
                'neigh_id': a_args.neigh_id,
            }
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params = data.params;
                if (params && params.redirect){
                    gotoObject(params.redirect);
                }
            }
        };
        sendApiPacket(args);
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
                getCurNeuronId(),
                {
                    'neigh_id': object_id
                }
            );
        } else if ('delete' == a_action){
            NeighDel(
                getCurNeuronId(),
                {
                    'neigh_id': object_id
                }
            );
        } else if ('add' == a_action){
            NeighAdd(
                getCurNeuronId(),
                {
                    'neigh_id': object_id
                }
            );
        } else if ('bookmark_add' == a_action){
            BookmarkAdd(
                getCurNeuronId(),
                {
                    'neigh_id': object_id
                }
            );
        } else if ('bookmark_del' == a_action){
            BookmarkDel(
                getCurNeuronId(),
                {
                    'neigh_id': object_id
                }
            );
        } else {
            alert('invalid action: "'+a_action+'"');
        }
    }

    function createCookie(name, value, days){
        var expires;

        if (days){
            var date = new Date();
            date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
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
        return null;
    }

    function eraseCookie(name){
        createCookie(name, "", -1);
    }

    var events_subscr       = {};
    var timer_events_subscr = null;
    function apiUpdateEventsSubscr(a_neuron_id, a_events){
        if (timer_events_subscr){
            clearInterval(timer_events_subscr);
            timer = null;
        }
        if (!events_subscr[a_neuron_id]){
            events_subscr[a_neuron_id] = a_events;
        } else {
            for (var key in a_events){
                events_subscr[a_neuron_id][key] = a_events[key];
            }
        }
        timer_events_subscr = setTimeout(
            function(){
                var events    = $.extend({}, events_subscr);
                events_subscr = {};

                var packets = [];
                for (var object_id in events){
                    var packet = {
                        'method':       'updateEventsSubscr',
                        'object_id':    object_id,    
                        'params':       {
                            'events':   events[object_id]
                        }
                    };
                    packets.push(packet);
                }

                // send request
                var args = {
                    'packets':  packets,
                    'success':  function(data){
                        var params = data.params;
                    },
                    'failed':   function(data){
                        var msg = data;
                        perror('Cannot update events subscr: '+msg);
                    }
                };
                sendApiPackets(args);
            },
            3000
        );
    }
</script>

