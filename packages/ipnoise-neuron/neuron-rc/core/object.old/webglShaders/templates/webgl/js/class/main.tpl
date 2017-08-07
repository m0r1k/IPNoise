$.widget(
    "ipnoise.IPNoiseWebglShaders",
    $.ipnoise.IPNoiseWebglObject,
{
    // default options
    options: {},

    // the constructor
    new: function(a_args){
        var self = this;

        // TODO
        self.element.attr('object_id', 'shaders');

        if (undefined == a_args.viewer){
            getLogger().pfatal("missing argument: 'viewer'"
                +" or it is not object",
                a_args
            );
        }

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglShaders');

        self.m_cur_shader           = undefined;
        self.m_cur_shader_class     = undefined;

        // init shaders
        for (var class_name in $.ipnoise){
            var res = /(IPNoiseWebglShader.+)/
                .exec(class_name);
            if (res){
                var shader_class = res[1];
                if ('IPNoiseWebglShaders' == shader_class){
                    // skip ourself
                    continue;
                }
                createWidget({
                    'class_name':   shader_class,
                    'args':         {
                        'viewer': self.m_viewer,
                    }
                });
            }
        }

        // do not forget return self!
        return self;
    },

    use: function(
        a_shader_class)
    {
        var self    = this;
        var res     = $('.'+a_shader_class);

        if (!res.length){
            self.getLogger().pfatal(
                "cannot use shader: '"+a_shader_class+"',"
                +" class not found",
                res
            );
        }

        for (var i = 0; i < res.length; i++){
            var el = $(res.get(i));
            if ('function' != jQuery.type(el[a_shader_class])){
                self.getLogger().pfatal(
                    "can not use shader: '"+a_shader_class+"',"
                    +" shader not found",
                    el
                );
            }
            var shader = el[a_shader_class]();
            if (!shader){
                self.getLogger().pfatal(
                    "can not use shader: '"+a_shader_class+"',"
                    +" shader not found",
                    shader_ptr
                );
            }
        }

        self.m_cur_shader       = shader;
        self.m_cur_shader_class = a_shader_class;

        shader.callWidgetMethod('use');

        return shader;
    },
});

