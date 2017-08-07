// vim:syntax=javascript tabstop=4 expandtab

// ONLY GENERIC STATIC FUNCTIONS
// FOR OBJECTS IN */js/class/main.tpl

var camera = 0;

function getCreateCamera(a_args)
{
    var args = {
        'params':   {},
        'success':  undefined,
        'failed':   undefined,
        'args':     {}
    };

    // copy args
    for (var key in a_args){
        args[key] = a_args[key];
    }

    var params = args.params;

    var packet = {
        'method':       'getCreateCamera',
        'params':       params,
        'success':      function(a_data){
            var params = a_data.params;
            if (params && params.html){
                var widgets = parseHtmlOnWidgets({
                    'html_base64':  params.html,
                    'args':         args.args
                });
                if (args.success){
                    args.success(a_data, widgets);
                }
            } else {
                if (args.failed){
                    args.failed(a_data);
                }
            }
        },
        'failed':       function(a_data){
            if (args.failed){
                args.failed(a_data);
            }
        }
    };

    var data = {
        'packet':   packet
    };
    sendApiPacket(data);
}

