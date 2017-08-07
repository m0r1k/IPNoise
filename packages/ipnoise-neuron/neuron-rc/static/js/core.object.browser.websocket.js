// ---------------- static ----------------

// ---------------- ObjectBrowserWebsocket ----------------

function ObjectBrowserWebsocket()
{
    var self = this;
}

ObjectBrowserWebsocket.prototype               = new Object;
ObjectBrowserWebsocket.prototype.getObjectType = function()
{
    return 'core.object.browser.websocket';
}

ObjectBrowserWebsocket.prototype.v_reset = function(
    a_args)
{
    var self = this;
    var url;

    var args = {
        'host':         '192.168.223.222',
        'port':         8080,
        'uri':          'api',
        'binary_type':  'arraybuffer',
        'onopen':       undefined,
        'onclose':      undefined,
        'onmessage':    undefined,
        'onerror':      undefined
    }
    copy_args(args, a_args);

    Object.prototype.v_reset.apply(self, [ args ]);

    self.m_host         = args.host;
    self.m_port         = args.port;
    self.m_uri          = args.uri;
    self.m_binary_type  = args.binary_type;
    self.m_onopen       = args.onopen;
    self.m_onclose      = args.onclose;
    self.m_onmessage    = args.onmessage;
    self.m_onopen       = args.onopen;

    url = 'ws://'+self.m_host+':'+self.m_port+'/'+self.m_uri;

    getLogger().pdebug(10, "open websocket to: "+url);

    self.m_ws = new WebSocket(url);
    if (!self.m_ws){
        self.getLogger().pfatal("cannot create WebSocket,"
            +" for url: '"+url+"'"
        );
    }

    if (undefined != self.m_binary_type){
        self.m_ws.binaryType = self.m_binary_type;
    }

    self.m_ws.onopen = function(a_ev){
        getLogger().pinfo(
            "websocket connection to: '"+url+"' opened"
        );
        if (self.m_onopen){
            self.m_onopen(self, a_ev);
        }
    }

    self.m_ws.onclose = function(a_ev){
        getLogger().pinfo(
            "websocket connection closed\n",
            a_ev
        );
        if (self.m_onclose){
            self.m_onclose(self, a_ev);
        }
    }

    self.m_ws.onmessage = function(a_ev){
        var arr = arrayBuffer_of_uint8_to_array_of_uint8(
            a_ev.data
        );
        var data = array_of_uint8_utf8_to_string_of_unicode(
            arr
        );

        if (self.m_onmessage){
            self.m_onmessage(self, a_ev, data);
        }
    }

    self.m_ws.onerror = function(a_ev){
        getLogger().pfatal(
            "websocket connection error",
            a_ev
        );

        if (self.onerror){
            self.onerror(self, a_ev);
        }
    }
}

ObjectBrowserWebsocket.prototype.send = function(
    a_data)
{
    var self        = this;
    var data_type   = typeof a_data;

    if ('string' == data_type){
        var ab = string_to_arrayBuffer_of_utf8_uint8(a_data);
        self.m_ws.send(ab);
    } else {
        self.getLogger().pfatal(
            "unsupported data type: '"+data_type+"'"
        );
    }

    // send
    //  var image = context.getImageData(0, 0, canvas.width, canvas.height);
    //  var buffer = new ArrayBuffer(image.data.length);
    //  var bytes = new Uint8Array(buffer);
    //  for (var i=0; i<bytes.length; i++) {
    //      bytes[i] = image.data[i];
    //  }
    //  websocket.send(buffer);

    //  recv
    //  var bytes = new Uint8Array(blob.size);
    //  var image = context.createImageData(canvas.width, canvas.height);
    //  for (var i=0; i<image.length; i++) {
    //      image[i] = bytes[i];
    //  }
    //  context.drawImage(image, 0, 0);
}

// ---------------- module_info ----------------

var module_info = {
    'type':         ObjectBrowserWebsocket.prototype.getObjectType(),
    'objectCreate': function(){
        return new ObjectBrowserWebsocket();
    }
};
module_register(module_info);

