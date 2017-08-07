// vim:syntax=javascript tabstop=4 expandtab

var g_debug = 10;
// g_logger in the end of file

function Logger(
    a_logger)
{
    var self = this;

    self.m_prefix = 'global';
}

Logger.prototype.setPrefix = function(
    a_prefix)
{
    var self = this;
    self.m_prefix = a_prefix;
}

function censor(censor)
{
    var i = 0;

    return function(key, value){
        if  (   0        !== i
            &&  'object' === jQuery.type(censor)
            &&  'object' ==  jQuery.type(value)
            &&  censor   ==  value)
        {
            return '[Circular]'; 
        }

        // seems to be a harded maximum
        // of 30 serialized objects?
        if (i >= 29){
            return '[Unknown]';
        }

        // so we know we aren't using
        // the original object anymore
        ++i; 

        return value;  
    }
}

Logger.prototype.plog = function(
    a_type,
    a_msg,
    a_arg)
{
    var self = this;

    var name        = '';
    var msg_console = '';
    var msg_body    = '';
    var dump        = '';

    if (self.m_prefix.length){
        name = ' '+self.m_prefix;
    }
    msg_console = '[ '+a_type+name+' ] '+a_msg;
    msg_body    = msg_console;

    if (3 <= arguments.length){
        dump = JSON.stringify(a_arg, censor(a_arg));
    }

    if ('string' == jQuery.type(dump)){
        msg_console += "\n";
        msg_console += dump;
    }

    if (undefined != console){
        console.log(msg_console);
    }

    if ('FATAL' == a_type){
        $('body').addClass('internal_error');
        $('body').html(
            '<h1>Internal error</h1>'
            +'<table>'
            +'<tr><td>Descr:</td><td>'+msg_body+'</td></tr>'
            +'<tr><td>Dump:</td><td><pre>'+dump+'</pre></td></tr>'
            +'</table>'
        );
    }
}

Logger.prototype.pinfo = function(
    a_msg,
    a_arg)
{
    var self = this;
    // TODO remove this hardcode from here
    // 20140729 morik@
    if (2 <= arguments.length){
        self.plog('INFO', a_msg, a_arg);
    } else {
        self.plog('INFO', a_msg);
    }
}

Logger.prototype.pwarn = function(
    a_msg,
    a_arg)
{
    var self = this;
    // TODO remove this hardcode from here
    // 20140729 morik@
    if (2 <= arguments.length){
        self.plog('WARNING', a_msg, a_arg);
    } else {
        self.plog('WARNING', a_msg);
    }
}

Logger.prototype.pfatal = function(
    a_msg,
    a_arg)
{
    var self = this;
    // TODO remove this hardcode from here
    // 20140729 morik@
    if (2 <= arguments.length){
        self.plog('FATAL', a_msg, a_arg);
    } else {
        self.plog('FATAL', a_msg);
    }
    throw {
        name:       'FatalError',
        message:    a_msg
    };
}

Logger.prototype.perror = function(
    a_msg,
    a_arg)
{
    var self = this;
    // TODO remove this hardcode from here
    // 20140729 morik@
    if (2 <= arguments.length){
        self.plog('ERROR', a_msg, a_arg);
    } else {
        self.plog('ERROR', a_msg);
    }
}

Logger.prototype.pdebug = function(
    a_level,
    a_msg,
    a_arg)
{
    var self = this;
    if (a_level < g_debug){
        // TODO remove this hardcode from here
        // 20140729 morik@
        if (3 <= arguments.length){
            self.plog('DEBUG '+a_level, a_msg, a_arg);
        } else {
            self.plog('DEBUG '+a_level, a_msg);
        }
    }
}

var g_logger = new Logger();

function getLogger()
{
    return g_logger;
}

