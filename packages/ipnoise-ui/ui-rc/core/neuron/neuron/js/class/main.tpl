var neurons_by_id = {};

// url
function getUrlParameter(name){
    var ret = decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(
            location.search
        )||[,null])[1]
    );
    return ret;
}

// neuron
function getCurNeuronId(a_event)
{
    var neuron_id = getUrlParameter('object_id');
    return neuron_id;
}

function getCurNeuron()
{
    var id      = getCurNeuronId();
    var neuron  = getNeuronById(id);
    return neuron;
}

function getNeuronById(a_id)
{
    return neurons_by_id[a_id];
}

function getNeuronClassName(a_el)
{
    var css_classes     = [];
    var obj_class_name;
    var neuron_class_name;

    // get css classes
    css_classes = a_el
        .attr('class')
        .split(/\s+/);

    // search first Neuron* class name
    for (var i = 0;
        i < css_classes.length;
        i++)
    {
        var res;
        var cur_css_class;
        cur_css_class = css_classes[i];

        res = /(Neuron[^\s]*)/.exec(
            cur_css_class
        );
        if (res){
            neuron_class_name = res[1];
            break;
        }
    }

    return neuron_class_name;
}

function getEvents()
{
    var packet = {
        method:     'getEvents',
        object_id:  getCurNeuronId()
    };
    var args = {
        packet:     packet,
        success:    function (data){
            var params  = data.params;
            var events  = params.events;
            for (var i = 0; i < events.length; i++){
                var event       = events[i];
                var object_id   = event.object_id;

                $("div[neuron_id='"+object_id+"']")
                    .each(function(){
                        var neuron_class_name;
                        var el = $(this);

                        neuron_class_name = getNeuronClassName(
                            el
                        );

                        // ok, get jquery object and inform
                        // about events
                        el[neuron_class_name](
                            'apiEvent',
                            event
                        );
                    });
            }
            console.log('MORIK api get events success');
            // wait new events
            getEvents();
        },
        failed:     function (data){
            console.log('MORIK api get events failed');
            // a little wait and try again (msec)
            setTimeout(getEvents, Math.random() * 5e3);
        }
    };

    sendApiPacket(args);
}

function NeuronClass()
{
    var self        = AbstractClass();
    var m_render_to = null;
    var m_view      = null;
    var m_props     = {};

    // Id
    self.setId = function(a_id){
        m_props._id         = a_id;
        neurons_by_id[a_id] = self;
    }
    self.getId = function(){
        return m_props._id;
    }

    // descr
    self.getDescr = function(){
        return m_props.descr;
    }
    self.setDescr = function(a_val){
        m_props.descr = a_val;
    }

    // name
    self.getName = function(){
        return m_props.name;
    }
    self.setName = function(a_val){
        m_props.name = a_val;
    }

    // render
    self.setRenderTo = function(a_render_to){
        m_render_to = a_render_to;
    }
    self.getRenderTo = function(){
        return m_render_to;
    }
    self.setView = function(a_view){
        m_view = a_view;
    }
    self.getView = function(){
        return m_view;
    }
    self.renderTo = function(a_el, a_view){
        var params = {};

        self.setRenderTo(a_el);
        self.setView(a_view);

        if (a_view){
            params.view = a_view;
        }
        var packet = {
            'method':   'render',
            'params':   params
        };
        var args = {
            'packet':     packet,
            'success':    function(data){
                var params  = data.params;
                var html    = Base64.decode(params.html);
                var el      = $(html);
                a_el.append(el);
            },
            'failed': function(data){}
        }
        self.sendApiPacket(args);
    }
    self.renderEmbeddedTo = function(a_el, a_view){
        var params = {};

        self.setRenderTo(a_el);
        self.setView(a_view);

        if (a_view){
            params.view = a_view;
        }
        var packet = {
            'method':   'renderEmbedded',
            'params':   params
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                var params  = data.params;
                var html    = Base64.decode(params.html);
                var el      = $(html);
                a_el.append(el);
            },
            'failed': function(data){}
        }
        self.sendApiPacket(args);
    }

    self.render = function(el){}

    // send api
    self.sendApiPacket = function(a_args){
        var req_id;
        if (!a_args.packet){
            a_args.packet = {};
        }
        if (!a_args.packet.object_id){
            a_args.packet.object_id = self.getId();
        }
        req_id = sendApiPacket(a_args);
        return req_id;
    }

    // load
    self.load = function(a_args){
        var packet = {
            'method':   'load'
        };
        var args = {
            'packet':   packet,
            'success':  function(data){
                m_props = data.params;
                if (    a_args
                    &&  a_args.success)
                {
                    // call success cb
                    a_args.success(data);
                }
            },
            'failed': function(data){
                if (    a_args
                    &&  a_args.failed)
                {
                    // call failed cb
                    a_args.failed(data);
                }
            }
        }
        // MORIK self.sendApiPacket(args);
    }

    // save
    self.save = function(a_el, a_event){
        if (!a_event){
            pfatal("missing argument 'a_event'");
        }

        // get target ui element
        var target = a_event.target || a_event.srcElement;

        // prevent default browser handler
        a_event.preventDefault();

        // stop event transmittion to upper elements
        a_event.stopPropagation();

        alert('save');
    };

    // delete
    self.delete = function(){
        var packet = {
            'method':   'delete'
        };
        var args = {
            'data':     packet,
            'success':  function(){
                var id = self.getId();
                $('div[id="'+id+'"]').remove();
                delete neurons_by_id[id];
            },
        }
        self.sendApiPacket(args);
    }

    self.deleteWithConfirm = function(){
        var res = confirm("Are You sure to delete address?");
        if (res){
            self.delete();
        }
    }

    // address
    self.isFullPage = function(){
        var ret         = 0;
        var id          = self.getId();
        var object_id   = getCurNeuronId();

        if (    id
            &&  object_id
            &&  object_id == id)
        {
            ret = 1;
        }

        return ret;
    };

    // events
    self.eventRerender = function(a_event){
        var el      = self.getRenderTo();
        var view    = self.getView();
        var id      = self.getId();

        // TODO
        el = $('div[id="'+id+'"]').parent();
        $('div[id="'+id+'"]').remove();
        self.renderEmbeddedTo(el, view);
    }

    self.processEvent = function(a_event){
        var type        = a_event.type;
        var object_id   = a_event.obj

        if ("rerender" == type){
            self.eventRerender(a_event);
        } else {
            perror("unsupported event type: '"+type+"'"
                +" object_id: '"+self.getId()+"'"
            );
        }
    }

    // bookmark
    self.bookmark = function(){
        var packet = {
            'method':   'bookmark'
        };
        var args = {
            'packet':   packet,
            'success':  function(){
                // remove icon
                $(document).find('.neuron_icon_bookmark').remove();
            },
        }
        self.sendApiPacket(args);

        return false;
    }

    // uiAddNew
    self.uiAddNew = function(a_event){
        if (!a_event){
            pfatal("missing argument 'a_event'");
        }

        // get target ui element
        var target = a_event.target || a_event.srcElement;

        // prevent default browser handler
        a_event.preventDefault();

        // stop event transmittion to upper elements
        a_event.stopPropagation();

        var map         = $(".neuron_user_map");
        var map_parent  = map.parent();

        map.remove();
        var library = NeuronClass();
        library.setId("god");
        library.renderEmbeddedTo(map_parent, 'map');
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

$.widget("neurons.Neuron", {
    // default options
    options: {
        have_render:    0,
        api_subscribe:  { 'all': 1 },

        // neuron
        neuron_id:      null,
        neuron_url:     null,

        // callbacks
        change: null
    },

    // the constructor
    _create: function(){
        this._init_options();

        if (!this.options.have_render){
            var neuron_class_name = this.getNeuronClassName();
            this.info = $("<h1>"+neuron_class_name+"</h1>")
                .appendTo(this.element);

            this._refresh();
        }

        if (this.options.api_subscribe){
            apiUpdateEventsSubscr(
                this.option('neuron_id'),
                this.options.api_subscribe
            );
        }
    },

    // called when created, and later when changing options
    _refresh: function(){
        // trigger a callback/event
        this._trigger( "change" );
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        perror("TODO unsubscribe from events");
        this.element.css("background-color", "transparent");
    },

    // _setOptions is called with a hash
    // of all options that are changing
    // always refresh when changing options
    _setOptions: function(){
        // _super and _superApply handle keeping
        // the right this-context
        this._superApply(arguments);
        this._refresh();
    },

    // _setOption is called for each individual option
    // that is changing
    _setOption: function(key, val){
        if ('neuron_descr' == key){
            $("textarea[name='descr']").text(val);
        }
        this._super(key, val);
    },

    // init element options from element attributes
    _init_options: function(){
        var attributes = this.element.getAttributes();
        for (var key in attributes){
            var val = attributes[key];
            var res = /^(neuron_.+)/.exec(key);
            if (res){
                var opt_name = res[1];
                this.option(opt_name, val);
            }
        }
    },
 
    apiEvent: function(a_event){
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        if ('props.changed' == type){
            var props = a_event.props;
            for (var prop_name in props){
                var prop_val = props[prop_name];

                if ('descr' == prop_name){
                    this.option(prop_name, prop_val);
                } else {
                    this.perror(
                        "Unsupported property: '"+prop_name+"'"
                        +" in event: '"+type+"'"
                    );
                }
            }
        } else {
            this.perror("Unsupported event: '"+type+"'");
        }
    },

    // send api
    sendApiPacket: function(a_args){
        var req_id;
        if (!a_args.packet){
            a_args.packet = {};
        }
        if (!a_args.packet.object_id){
            a_args.packet.object_id = this.option('neuron_id');
        }
        req_id = sendApiPacket(a_args);
        return req_id;
    },

    getNeuronClassName: function(){
        var ret = getNeuronClassName(this.element);
        return ret;
    },

    // log methods
    perror: function(a_msg){
        var neuron_id   = this.option('neuron_id');
        var class_name  = this.getNeuronClassName();
        perror('['+class_name+','+neuron_id+'] '+a_msg);
    },
    pwarn: function(a_msg){
        var neuron_id = this.option('neuron_id');
        pwarn('['+class_name+','+neuron_id+'] '+a_msg);
    },
    pinfo: function(a_msg){
        var neuron_id = this.option('neuron_id');
        pinfo('['+class_name+','+neuron_id+'] '+a_msg);
    },
    pdebug: function(a_level, a_msg){
        var neuron_id = this.option('neuron_id');
        pdebug(a_level,
            '['+class_name+','+neuron_id+'] '+a_msg
        );
    },

});

// -------------------------- init --------------------------

$(document).ready(function(){
    // load events
    getEvents();
});

