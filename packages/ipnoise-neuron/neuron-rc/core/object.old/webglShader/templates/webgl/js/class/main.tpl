// vim:syntax=javascript tabstop=4 expandtab

$.widget(
    "ipnoise.IPNoiseWebglShader",
    $.ipnoise.IPNoiseWebglObject,
{
    // default options
    options: {},

    // the constructor
    new: function(a_name, a_args){
        var self = this;

        if (undefined == a_name){
            getLogger().pfatal("missing argument: 'name'");
        }

        if ('object' != jQuery.type(a_args.viewer)){
            getLogger().pfatal("missing argument: 'viewer'"
                +" or it is not object",
                a_args
            );
        }

        self._super(a_args);

        self.m_logger = new Logger();
        self.m_logger.setPrefix('IPNoiseWebglShader');

        self.m_name         = a_name;
        self.m_attrs        = {};
        self.m_uniforms     = {};
        self.m_program      = undefined;

        // do not forget return self!
        return self;
    },

    setViewer: function(a_viewer){
        var self = this;
        self.m_viewer = a_viewer;
    },

    getViewer: function(){
        var self = this;
        return self.m_viewer;
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

    getGl: function(){
        var self = this;
        var gl   = self.m_viewer.callWidgetMethod(
            'getGl'
        );
        return gl;
    },

    getName: function(){
        var self = this;
        return self.m_name;
    },

    setupAttributes: function(){
        var self = this;
        var gl   = self.getGl();

        for (var i = 1; i <= g_max_shaders_index; i++){
            // OpenGL requires attribute zero
            // to be enabled otherwise
            // it will not render anything
            gl.disableVertexAttribArray(i);
        }

        for (var attr_name in self.m_attrs){
            var index = self.m_attrs[attr_name];
            gl.enableVertexAttribArray(index);
        }
    },

    initLocations: function(a_args){
        var self = this;
        var gl   = self.getGl();

        var args = {
            attrs:      [],
            uniforms:   []
        };

        // copy args
        for (var key in a_args){
            args[key] = a_args[key];
        }

        var attrs    = args.attrs;
        var uniforms = args.uniforms;

        for (var i = 0; i < attrs.length; i++){
            var cur_attr = attrs[i];
            var index    = gl.getAttribLocation(
                self.m_program,
                cur_attr
            );
            if (0 > index){
                self.getLogger().pfatal(
                     "cannot get attr: '"+cur_attr+"' location"
                );
            }

            // store attr index to shader
            self.m_attrs[cur_attr] = index;

            if (index > g_max_shaders_index){
                g_max_shaders_index = index;
            }

            // enable attribute
            gl.enableVertexAttribArray(index);

            self.getLogger().pdebug(
                10,
                   "enabling attr: '"+cur_attr+"'"
                +" location ("+index+")"
            );
        }

        // uniforms
        for (var i = 0; i < uniforms.length; i++){
            var cur_uniform = uniforms[i];
            var obj         = gl.getUniformLocation(
                self.m_program,
                cur_uniform
            );
            if (!obj){
                self.getLogger().pfatal(
                 "cannot get uniform: '"+cur_uniform+"'"
                    +" location,"
                    +" for shader: '"+self.getName()+"'"
                );
            }

            self.m_uniforms[cur_uniform] = obj;

            self.getLogger().pdebug(
                10,
                "found uniform: '"+cur_uniform+"'"
                +" location,"
                +" for shader: '"+self.getName()+"'"
            );
        }
    },

    getProgram: function(){
        var self = this;
        return self.m_program;
    },

    getAttrs: function(){
        var self = this;
        return self.m_attrs;
    },

    getUniforms: function(){
        var self = this;
        return self.m_uniforms;
    },

    use: function(){
        var self = this;
        var gl   = self.getGl();
        gl.useProgram(self.m_program);
        self.setupAttributes();
    },

});

