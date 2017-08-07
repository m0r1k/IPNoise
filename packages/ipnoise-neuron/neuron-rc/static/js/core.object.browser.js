// ---------------- static ----------------

// ---------------- ObjectBrowser ----------------

function ObjectBrowser()
{
    var self = this;
}

ObjectBrowser.prototype                  = new Object;
ObjectBrowser.prototype.getObjectType    = function()
{
    return 'core.object.browser';
}

ObjectBrowser.prototype.v_reset = function(
    a_args)
{
    var self    = this;
    var args    = {
        'document': undefined
    };
    copy_args(args, a_args);

    if (!args.document){
        self.getLogger().pfatal(
            "missing argument: 'document'"
        );
    }

    // init super
    Object.prototype.v_reset.apply(self, [ args ]);

    var doc = $(args.document);

    self.m_keyboard = object_create(
        'core.object.browser.keyboard',
        {
            'document': args.document
        }
    );

    self.m_websocket = object_create(
        'core.object.browser.websocket',
        {
            'document':     args.document,
            'onopen':       self.connection_opened,
            'onmessage':    function(a_ws, a_ev, a_data){
/*
                var req = object_create(
                    'core.object.browser.http.request',
                    {
                        'data': a_data
                    }
                );
*/
                console.log("morik: '"+a_data+"'");
            }
        }
    );
}

ObjectBrowser.prototype.connection_opened = function(
    a_ws,
    a_args)
{
    var data = "бугага привед медвед\n";

    a_ws.send("ACTION /self/info_get IPNOISE/1.0\r\n");
}

// ---------------- module_info ----------------

var module_info = {
    'type':         ObjectBrowser.prototype.getObjectType(),
    'objectCreate': function(){
        return new ObjectBrowser();
    }
};
module_register(module_info);

