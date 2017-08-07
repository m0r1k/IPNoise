var g_debug = 100;
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

Logger.prototype.plog = function(
    a_type,
    a_msg,
    a_arg)
{
    var self = this;

    var name = '';
    var msg  = '';
    var dump = '';

    if (self.m_prefix.length){
        name = ' '+self.m_prefix;
    }
    msg = '[ '+a_type+name+' ] '+a_msg;

    for (var key in a_arg){
        var val = a_arg[key];
        dump += '   ' + key +': '+val+"\n";
    }

    if (!dump){
        dump = a_arg;
    }

    if (3 <= arguments.length){
        msg += "\n";
        msg += dump;
    }

    console.log(msg);
}

Logger.prototype.pinfo = function(
    a_msg,
    a_arg)
{
    var self = this;
    // TODO remove this hardcode from here
    // 20140729 morik@
    if (3 <= arguments.length){
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
    if (3 <= arguments.length){
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
    if (3 <= arguments.length){
        self.plog('FATAL', a_msg, a_arg);
    } else {
        self.plog('FATAL', a_msg);
    }
    throw {
        name:       'FatalError',
        message:    a_msg
    };
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

