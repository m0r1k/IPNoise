// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseHtmlObjectShop",
    $.ipnoise.IPNoiseHtmlObject,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // prevent double click to select text
        self.element.disableSelection();

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseHtmlObjectShop');

        self.element.append(''
            +'<center>'
               +'<h1>Toscana</h1>'
               +'<perl>
                    my $ret         = '';
                    my $img         = '<img'
                        .' width="900px"'
                        .' src="/static/images/Amalfi_Italy_4.jpg"'
                        .'/>';
                    my %args = (
                        title => $img
                    );
                    $ret .= getHref(%args);
                    return $ret;
                </perl>'
            +'</center>'
        );

        // do not forget return self!
        return self;
    },

    // events bound via _on are removed automatically
    // revert other modifications here
    _destroy: function(){
        var self = this;
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
        self._super(a_key, a_val, a_args);
    },

    apiEvent: function(a_event){
        var self        = this;
        var type        = a_event.type;
        var object_id   = a_event.object_id;

        self.getLogger().perror(
            "IPNoiseHtmlObjectShop api event,"
            +" type: '"      + type      + "',"
            +" object_id: '" + object_id + "'"
        );

        // may be super event?
        self._super(a_event);
    }
});

