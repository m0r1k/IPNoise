// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseHtmlObject",
    $.ipnoise.IPNoiseObject,
{
    // default options
    options: {
        api_subscribe:  { 'all': 1 },

        // object
        object_id:      undefined,
        object_url:     undefined,

        // callbacks
        change:         undefined
    },

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseHtmlObject');

        // do not forget return self!
        return self;
    },

    // called when created, and later when changing options
    _refresh: function(){
        var self = this;
        // trigger a callback/event
        self._trigger( "change" );
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;
        self.getLogger().perror("TODO unsubscribe from events");
        self.element.css("background-color", "transparent");
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
        self._super(a_key, a_val, a_args);
    },

    apiEvent: function(a_event){
        var self = this;

        // may be super event?
        self._super(a_event);
    },

    draw: function(){
        var ipnoise_class_name = self.getIPNoiseClassName();
        self.info = $("<h1>"+ipnoise_class_name+"</h1>")
            .appendTo(self.element);

        self._refresh();
    }
});

