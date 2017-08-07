// ---------------- static ----------------

var g_logger = undefined;

function getLogger()
{
    if (!g_logger){
        g_logger = new Logger();
    }

    return g_logger;
}

// ---------------- Logger ----------------

function Logger()
{
    var self = this;

    self.m_debug_level = 10;
}

Logger.prototype.plog = function(
    a_type,
    a_msg,
    a_args)
{
    var self = this;

    var line = "[ " + a_type + " ] " + a_msg;
    if (console){
        console.log(line);
        if (3 <= arguments.length){
            console.log(a_args);
        }
    }

    if ('FATAL' == a_type){
        alert(line);
        throw({ what: line });
    }
}

Logger.prototype.getDebugLevel = function()
{
    var self = this;

    return self.m_debug_level;
}

Logger.prototype.pdebug = function(
    a_level,
    a_msg,
    a_args)
{
    var self = this;

    if (a_level <= self.getDebugLevel()){
        if (3 <= arguments.length){
            self.plog('DEBUG '+ a_level, a_msg, a_args);
        } else {
            self.plog('DEBUG '+ a_level, a_msg);
        }
    }
}

Logger.prototype.pinfo = function(
    a_msg,
    a_args)
{
    var self = this;

    if (3 <= arguments.length){
        self.plog('INFO', a_msg, a_args);
    } else {
        self.plog('INFO', a_msg);
    }
}

Logger.prototype.perror = function(
    a_msg,
    a_args)
{
    var self = this;

    self.plog('ERROR', a_msg, a_args);
}

Logger.prototype.pfatal = function(
    a_msg,
    a_args)
{
    var self = this;

    self.plog('FATAL', a_msg, a_args);
}

Logger.prototype.pwarn = function(
    a_msg,
    a_args)
{
    var self = this;

    self.plog('WARNING', a_msg, a_args);
}

