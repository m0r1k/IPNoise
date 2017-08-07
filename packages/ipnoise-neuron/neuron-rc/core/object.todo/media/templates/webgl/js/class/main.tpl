// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseWebglObjectMedia",
    $.ipnoise.IPNoiseWebglObject,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        if (undefined == a_args.viewer){
            getLogger().pfatal("missing argument: 'viewer'");
        }

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglObjectMedia');

        // do not forget return self!
        return self;
    },

    // ---------------- api ----------------

    animate: function(){
        var self = this;
        self._super();
    },

    draw: function(a_pick_color){
        var self = this;
        self._super(a_pick_color);
    },

    select: function(){
        var self = this;
        self._super();
    },

    deSelect: function(){
        var self = this;
        self._super();
    },

    handleKeys: function(
        a_currently_pressed_keys)
    {
        var self = this;
        self._super(a_currently_pressed_keys);
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;
        // remove generated elements
        self._super();
    },

    _refresh: function(){
        var self = this;
        self._super();
    },

    // _setOptions is called with a hash
    // of all options that are changing
    // always refresh when changing options
    _setOptions: function(){
        var self = this;
        // _super and _superApply handle keeping
        // the right this-context
        self._superApply(arguments);
        self._refresh();
    },

    // _setOption is called for each individual option
    // that is changing
    _setOption: function(
        a_key,
        a_val,
        a_args)
    {
        var self = this;
        var args = {};
        copy_args(args, a_args);

        if ('object_status' == a_key){
            args.handled = 1;
        }

        self._super(a_key, a_val, args);
    },

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        if ('objects.changed' == type){
            var objects = a_event.objects;
        }

        // may be super event?
        self._super(a_event);
    },
});

