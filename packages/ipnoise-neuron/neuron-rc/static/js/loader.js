// ---------------- static ----------------

var g_loader = undefined;

function getLoader()
{
    if (!g_loader){
        g_loader = new Loader();
    }

    return g_loader;
}

// ---------------- Loader ----------------

function Loader()
{
    var self = this;

    self.m_queue_waiting    = [];
    self.m_queue_loading    = [];
    self.m_onload           = undefined;
}

Loader.prototype.onload = function(
    a_cb)
{
    var self = this;

    self.m_onload = a_cb;
}

Loader.prototype.flush = function()
{
    var self = this;

    do {
        if (self.m_queue_loading.length){
            // already something loading..
            break;
        }

        var resource_info = self.m_queue_waiting.shift();
        if (!resource_info){
            // all loaded
            if (self.m_onload){
                self.m_onload();
            }
            break;
        }

        // load
        self.m_queue_loading.push(resource_info);

        var resource_type   = resource_info.type;
        var resource_src    = resource_info.src;
        var resource_target = resource_info.target;

        if (undefined == resource_src){
            getLogger().pfatal("missing resource src",
                resource_info
            );
        }
        if (undefined == resource_target){
            getLogger().pfatal("missing resource target",
                resource_info
            );
        }

        if ('script' == resource_type){
            var el = $("<script>");
            el.attr('type', 'text/javascript');
            $('head').children().last().after(el);
            el.attr('src', resource_src);
            el.load(function(){
                self.m_queue_loading = [];
                self.flush();
            });
            el.error(function(){
                getLogger().pfatal(
                    "cannot load: '"+resource_src+"'"
                );
            });
        } else {
            getLogger().pfatal("unsupported"
                +" resource type: '"+resource_type+"'"
            );
        }
    } while (0);
}

Loader.prototype.load = function(
    a_info)
{
    var self = this;

    self.m_queue_waiting.push(a_info);
    self.flush();
}

