var g_inspect_for = undefined;

function inspector_mouseout(a_ev)
{
    var self    = this;
    var target  = $(a_ev.target);

    target.removeClass("editor_inspect");
    target.unbind('mouseout', inspector_mouseout);
}

function inspector_mouseclick(a_ev)
{
    var target    = $(a_ev.target);
    var object_el = $('#'+g_inspect_for);
    var widget_el = object_el.find('.ipnoise_widget');
    var editor    = widget_el.data('custom-IPNoiseWidgetEditor');

    var inspect_object = target.closest('.ipnoise_object');
    if (inspect_object){
        var inspect_id = inspect_object.attr('id');
        editor.inspectStop(inspect_id);
    }
}

function inspector_mousemove(a_ev)
{
    var self    = this;
    var target  = $(a_ev.target);


    if (target.closest('.ipnoise_widget')){
        target.addClass("editor_inspect");
        target.mouseout(inspector_mouseout);
    }
}

function inspector_keyup(a_ev)
{
    var target  = $(a_ev.target);
    var editor  = $('#'+g_inspect_for);

    var self    = editor.data('custom-IPNoiseWidgetEditor');

    if (KeyEvent.DOM_VK_ESCAPE == a_ev.keyCode){
        self.inspectStop();
    }
};

// ---------------- widget ----------------

$.widget("custom.IPNoiseWidgetEditor", $.custom.IPNoiseWidget, {
    // default options
    options: {
        id:         undefined,
        target_id:  '',
        title:      'Editor'
    },
    _create: function(a_args){
        var self = this;

        self._super();

        var id = self.getObjectId();

        self.element.addClass('ipnoise_editor');
        self.element.html('<div class="dialog">'
            +'<div class="tabs">'

                +'<h3>Create new</h3>'
                +'<div id="'+id+'-tabs-create" class="tabs-create tab">'
                    +'Name:  <input class="new_name"  type="text"></input><br/>'
                    +'Pos X: <input class="new_pos_x" type="text"></input><br/>'
                    +'Pos Y: <input class="new_pos_y" type="text"></input><br/>'
                    +'Pos Z: <input class="new_pos_z" type="text"></input><br/>'
                    +'<button>Create</button>'
                +'</div>'

                +'<h3>Select existen</h3>'
                +'<div id="'+id+'-tabs-info" class="tabs-info tab">'
                    +'<a>'
                        +'<img height="25px"'
                            +'src="/static/images/search_001.png">'
                        +'</img>'
                    +'</a>'
                    +'<br/>'
                    +'Target ID: '
                        +'<input type="text"'
                            +' class="target_id">'
                        +'</input>'
                +'</div>'

                +'<h3>Props</h3>'
                +'<div id="'+id+'-tabs-props" class="tabs-props tab">'
                    +'<div class="select-editable">'
                        +'<select onchange="this.nextElementSibling.value=this.value">'
                            +'<option></option>'
                        +'</select>'
                        +'<input type="text" name="format" value="" />'
                    +'</div>'
                    +'<textarea disabled></textarea>'
                    +'<button disabled></button>'
                +'</div>'

                +'<h3>Actions</h3>'
                +'<div id="'+id+'-tabs-actions" class="tabs-actions tab">'
                    +'<div style="overflow: auto;">'
                        +'<table width="100%">'
                            +'<tr>'
                                +'<th>from</th>'
                                +'<th>to</th>'
                                +'<th>type</th>'
                            +'</tr>'
                        +'</table>'
                    +'</div>'
                    +'<div>'
                        +'<form action="">'
                            +'<input class="action_type"></input>'
                            +'<input class="action_data"></input>'
                            +'<button>Send</button>'
                        +'</form>'
                    +'</div>'
                +'</div>'

            +'</div>'
            +'</div>'
        );

        var tab_create  = self.element.find('#'+id+'-tabs-create');
        var tab_info    = self.element.find('#'+id+'-tabs-info');
        var tab_props   = self.element.find('#'+id+'-tabs-props');
        var tab_actions = self.element.find('#'+id+'-tabs-actions');

        // create
        tab_create.find('button').click(self.createNewObject);

        // info
        tab_info.find('a').click(self.inspectStart);

        // props
        tab_props.find('select').change(self.propSelect);
        tab_props.find('button')
            .text('Create')
            .click(self.propEdit);
        tab_props.find('input')
            .keyup(self.propNameChanged);

        // actions
        tab_actions.find('form').submit(
            self.actionsSend
        );
        tab_actions.find('.action_type').val('actionPosSet');
        tab_actions.find('.action_data').val(JSON.stringify(
            { x: 100, y: 100, z: 100 }
        ));

        //alert(self.element.get(0).outerHTML);
        self.element.find('.dialog').dialog({
            title:      self.options.title,
            width:      self.options.width,
            height:     self.options.height,
            appendTo:   self.element
        });

        // disable all tabs except first
        self.element
            .find('.tabs')
            .accordion();

    },

    _setOption: function(a_key, a_val){
        var self = this;
        self._super(a_key, a_val);

        if ('target_id' == a_key){
            var id       = self.getObjectId();
            var tab_info = self.element.find(
                '#'+id+'-tabs-info'
            );
            tab_info.find('.target_id').val(a_val);
        }
    },

    on_action: function(a_socket, a_action){
        var self = this;
        var from = a_action.from;
        var to   = a_action.to;
        var type = a_action.type;
        var line = from + ' -> ' + to + ' type: ' + type;

        var tab_props = self.element
            .find('.tabs-props');

        self.element
            .find('.tabs-actions')
            .find('table')
            .append($(
                '<tr>'
                +'<td>'+from+'</td>'
                +'<td>'+to+'</td>'
                +'<td>'+type+'</td>'
                +'</tr>'
            ));

        if (a_action.error){
            self.setError(a_action.error);
            getLogger().perror(
                "received error",
                a_action.error
            );
            return;
        }

        if ('actionPropsNamesGetDone' == type){
            var props  = a_action.data;
            var select = tab_props.find('select');

            select.empty().append(
                $('<option></option>')
            );

            for (var i = 0; i < props.length; i++){
                var prop_name = props[i];
                select.append(
                    $('<option>'+prop_name+'</option>')
                );
            }

        } else if ('actionPropGetDone' == type){
            var prop_name = a_action.name;
            var prop_data = a_action.data;
            var data      = '';

            tab_props.find('textarea').val(prop_data);
            tab_props.find('button').removeAttr('disabled');

            self.setInfo('Loaded');
        } else if ('actionPropSetDone' == type){
            tab_props.find('button')
                .unbind('click')
                .text('Edit')
                .click(self.propEdit);

            tab_props.find('textarea').attr(
                'disabled', ''
            );

            self.setInfo('Saved');
        }

        self._super(a_socket, a_action);
    },

    setInspectObjectId: function(a_id){
        var self        = this;
        var id          = self.getObjectId();
        var tab_info    = self.element.find('#'+id+'-tabs-info');
        var tab_props   = self.element.find('#'+id+'-tabs-props');
        var widget      = $('#'+a_id);

        // update options
        self.option('target_id', a_id);

        // update info tab
        tab_info.find('img').attr('src',
            '/static/images/search_001.png'
        );

        // update props tab
        tab_props.find('textarea')
            .val('')
            .attr('disabled', '');

        tab_props.find('button')
            .val('Edit')
            .attr('disabled', '');

        // enable tabs
        self.element.find('.tabs').tabs(
            'option', 'disabled', false
        );
    },

    inspectStop:    function(a_id){
        var self = this;
        var tab  = self.element.find(
            '#'+g_inspect_for+'-tabs-info'
        );

        var doc = self.getDocument();

        // remove handlers
        doc.find('.editor_inspect')
           .removeClass('editor_inspect');

        doc.unbind('click',     inspector_mouseclick);
        doc.unbind('mousemove', inspector_mousemove);
        doc.unbind('keyup',     inspector_keyup);

        // update icon
        tab.find('img').attr('src',
            '/static/images/search_001.png'
        );

        // update id if found
        if (a_id){
            self.setInspectObjectId(a_id);
        }

        g_inspect_for = undefined;
    },

    inspectStart:   function(a_ev){
        var target      = $(a_ev.target);
        var tab         = target.closest('.tab');
        var editor      = tab.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');
        var editor_id   = self.getObjectId();

        // if another editor trying inspect widget,
        // then stop it
        if (g_inspect_for){
            self.inspectStop();
            g_inspect_for = undefined;
        }

        // create new inspector
        g_inspect_for = editor_id;

        $('#'+editor_id+'-tabs-info').find('img').attr(
            'src',
            '/static/images/search_003.png'
        );

        var doc = self.getDocument();

        doc.click(inspector_mouseclick);
        doc.mousemove(inspector_mousemove);
        doc.keyup(inspector_keyup);

        return false;
    },

    propNameChanged: function(a_ev){
        var target = $(a_ev.target);
        var val    = target.val();
        var tab    = target.closest('.tab');

        if (val){
            //tab.find('textarea').removeAttr('disabled');
            tab.find('button').removeAttr('disabled');
        } else {
            tab.find('textarea').attr('disabled', '');
            tab.find('button').attr('disabled',   '');
        }
    },

    propEdit: function(a_ev){
        var target      = $(a_ev.target);
        var tab         = target.closest('.tab');
        var editor      = tab.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');

        var select      = tab.find('select');
        var target_id   = self.option('target_id');
        var name        = select.val();

        tab.find('textarea').removeAttr('disabled');
        tab.find('button')
            .unbind('click')
            .text('Save')
            .click(self.propSave)
            .removeAttr('disabled');

        return false;
    },

    setStatus: function(a_msg, a_timeout){
        var self    = this;
        var title   = self.option('title');
        var line    = title;

        if (a_msg){
            line += ' (' + a_msg + ')';
        }

        self.element.find('.dialog').dialog(
            'option', 'title', line
        );

        if (a_timeout){
            setTimeout(
                function(){
                    self.element.find('.dialog').dialog(
                        'option', 'title', title
                    );
                },
                a_timeout
            );
        }
    },

    setInfo: function(a_msg, a_timeout){
        var self    = this;
        var timeout = a_timeout ? a_timeout : 2000;
        self.setStatus(a_msg, timeout);
    },

    setError: function(a_msg, a_timeout){
        var self = this;
        var timeout = a_timeout ? a_timeout : 5000;
        self.setStatus(a_msg, timeout);
    },

    propSelect: function(a_ev){
        var target      = $(a_ev.target);
        var tab         = target.closest('.tab');
        var editor      = tab.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');

        var select      = tab.find('select');
        var target_id   = self.option('target_id');
        var prop_name   = select.val();

        tab.find('textarea').val('');

        if (prop_name){
            self.setInfo('Loading..');
            tab.find('button').text('Edit');
            self.emit('actionPropGet', {
                'to':   target_id,
                'name': prop_name
            });
        } else {
            tab.find('button').text('Create');
            //tab.find('textarea').attr('disabled', '');
            //tab.find('button').attr('disabled',   '');
        }
    },

    propSave: function(a_ev){
        var target      = $(a_ev.target);
        var tab         = target.closest('.tab');
        var editor      = tab.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');

        var target_id   = self.option('target_id');
        var input       = tab.find('input');
        var code        = tab.find('textarea').val();
        var prop_name   = input.val();

        self.setInfo("Saving..");

        if (!prop_name){
            getLogger().pfatal('prop_name is empty');
        }

        self.emit('actionPropSet', {
            'to':       target_id,
            'name':     prop_name,
            'data':     code
        });

        //tab.find('textarea').attr('disabled', '');
        //tab.find('button').attr('disabled', '');

        return false;
    },

    actionsSend: function(a_ev){
        var target      = $(a_ev.target);
        var tab         = target.closest('.tab');
        var action_type_el = tab.find('.action_type');
        var action_data_el = tab.find('.action_data');
        var editor      = tab.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');
        var widget_id   = self.getObjectId();
        var target_id   = self.option('target_id');
        var action_type = action_type_el.val();
        var action_data = action_data_el.val();

        var msg = {
            'from':     widget_id,
            'to':       target_id
        }

        try {
            var obj = jQuery.parseJSON(action_data);
            $.extend(msg, obj);
        } catch (a_ev){
            self.setError('cannot parse action data');
            self.perror("cannot parser action data", {
                'action_type':  action_type,
                'action_data':  action_data,
                'widget_id':    widget_id,
                'target_id':    target_id,
                'error':        a_ev
            });
        }

        self.emit(action_type, msg);

        //input.val('');

        return false;
    },

    tabSelected: function(a_ev, a_ui){
        var target      = $(a_ev.target);
        var tab_id      = a_ui.newTab.find('a').attr('href');
        var tab         = $(tab_id);
        var select      = tab.find('select');

        var editor      = tab.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');
        var target_id   = self.option('target_id');

        self.setInfo('Load props names');

        self.emit('actionPropsNamesGet', {
            'to': target_id
        });
    },

    createNewObject: function(a_ev){
        var target      = $(a_ev.target);
        var tab_create  = target.closest('.tab');
        var editor      = tab_create.closest('.ipnoise_editor');
        var self        = editor.data('custom-IPNoiseWidgetEditor');
        var new_name    = tab_create.find('.new_name');
        var new_pos_x   = tab_create.find('.new_pos_x');
        var new_pos_y   = tab_create.find('.new_pos_y');
        var new_pos_z   = tab_create.find('.new_pos_z');
        var new_pos     = {
            'x': new_pos_x.val(),
            'y': new_pos_y.val(),
            'z': new_pos_z.val()
        };

        self.emit('actionObjectCreate', {
            'to':   'server',
            'name': new_name.val(),
            'pos':  new_pos
        });
    }
});

