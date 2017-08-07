$.widget('custom.IPNoiseObject', $.Widget, {
    // default options
    options: {
        'id':       undefined,
        'pos':      { 'x': 0, 'y': 0, 'z': 0 },
        'type':     'ipnoise_object'
    },

    _create: function(){
        var self    = this;
        var id      = self.option('id');
        var type    = self.option('type');

        self.element.attr('id', id);
        self.element.addClass(type);


/*
        g_session.on('from_server', function(a_action){
            // this - Socket
            var to      = a_action.to;
            var type    = a_action.type;

            if (to == id){
                var self_el = $('#'+to);
                var self    = self_el.data(
                    'custom-IPNoiseObject'
                );

                if ('actionRender' == type){
                    self.render();
                } else if ('actionRenderHtml' == type){
                    self.element.empty();
                    try {
                        eval(a_action.data);
                    } catch (a_err){
                        getLogger().pfatal(
                            "cannot render object: '"+id+"'",
                            a_err
                        );
                    }
                    console.log(a_action.pos);
                    if (a_action.pos){
                        self.option('pos', a_action.pos);
                    }
                } else if ( 'actionPropGetDone' == type
                    ||      'actionPropChanged' == type)
                {
                    var prop_name  = a_action.name;
                    var prop_data  = a_action.data;

                    self.option(prop_name, prop_data);
                }

                var widget = self.getWidget();
                if (widget){
                    widget.on_action(g_session, a_action);
                }
            }
        });
*/
        self.render();
    },

    _setOption: function(a_key, a_val){
        var self = this;
        self._super(a_key, a_val);

        if ('pos' == a_key){
            self.element.css('left',    a_val.x);
            self.element.css('top',     a_val.y);
            self.element.css('z-index', a_val.z);
        }
    },

    _setOptions: function(a_options){
        var self = this;
        self._super(a_options);
    },

    _on_click: function(a_ev){
        var target  = $(a_ev.target);
        var widget  = target.closest('.ipnoise_widget');
        var self    = widget.data('custom-IPNoiseWidget');
        var code    = self.option('on_click');

        eval(code);
    },

    getObjectId: function(){
        var self = this;
        var id   = self.option('id');
        return id;
    },

    getDocument: function(){
        var doc = window.opener.document
            || window.parent.document
            || window.document;

        return $(doc);
    },

    render: function(){
        var self = this;
        var id   = self.option('id');

        console.log('render '+id);

        g_viewer.emit('actionRenderHtml', {
            'from': id,
            'to':   id
        });

/*
        var code;

        self.element.html('');
        self.element.addClass('ipnoise_widget');
        self.element.css('top',  self.options.top  + 'px');
        self.element.css('left', self.options.left + 'px');

        //self.element.html(self.options.code);

        self.element.draggable();
        self.element.resizable();

        self.element.unbind('click', self._on_click);
        code = self.option('on_click');
        if (code){
            self.element.click(self._on_click);
        }

        eval(self.options.on_render);
*/
    },

    getWidgetElement: function(){
        var self = this;
        var el   = self.element.find('.ipnoise_widget');
        return el;
    },

    getWidget: function(){
        var self      = this;
        var widget_el = self.getWidgetElement();
        var nses      = widget_el.data();
        var widget    = undefined;

        for (var ns in nses){
            var res = /^(custom-IPNoise.+)/.exec(ns);
            if (    res
                &&  2 <= res.length)
            {
                widget = widget_el.data(res[1]);
                break;
            }
        }

        return widget;
    },

    emit: function(a_type, a_args){
        var self = this;
        var args = {
            'from': self.getObjectId()
        };

        $.extend(args, a_args);

        g_viewer.emit(a_type, args);
    },

    perror: function(a_msg, a_data){
        var self = this;

        getLogger().perror(a_msg, a_data);
    }
});

