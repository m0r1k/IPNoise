$.widget(
    "ipnoise.IPNoiseHtmlObjectPhoto",
    $.ipnoise.IPNoiseHtmlObject,
{
    // the constructor
    new: function(a_args){
        var self = this;

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseHtmlObjectPhoto');

        // do not forget return self!
        return self;
    }
});

