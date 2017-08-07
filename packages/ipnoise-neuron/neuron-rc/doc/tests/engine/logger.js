var s_assert = require('assert');
var s_extend = require('extend');

function Logger(a_args)
{
    var self    = this;
    var args    = {
        'prefix':       '',
        'debug_level':  100
    };
    s_extend(args, a_args);

    self.m_prefix       = args.prefix;
    self.m_debug_level  = args.debug_level;
}

Logger.prototype.plog = function(a_type, a_msg, a_data)
{
    var self = this;

    var line = '[ ' + a_type + ' ] ' + self.m_prefix + a_msg;

    console.log(line);

    if (2 < arguments.length){
        console.log(a_data);
    }

    if ('FATAL' == a_type){
        s_assert.equal(0, 1);
    }
}

Logger.prototype.pfatal = function(a_msg, a_data)
{
    var self = this;

    if (1 < arguments.length){
        self.plog('FATAL', a_msg, a_data);
    } else {
        self.plog('FATAL', a_msg);
    }
}

Logger.prototype.pdebug = function(a_level, a_msg, a_data)
{
    var self = this;

    if (self.m_debug_level < a_level){
        if (1 < arguments.length){
            self.plog('DEBUG', a_msg, a_data);
        } else {
            self.plog('DEBUG', a_msg);
        }
    }
}

Logger.prototype.pwarn = function(a_msg, a_data)
{
    var self = this;

    if (1 < arguments.length){
        self.plog('WARNING', a_msg, a_data);
    } else {
        self.plog('WARNING', a_msg);
    }
}

Logger.prototype.pinfo = function(a_msg, a_data)
{
    var self = this;

    if (1 < arguments.length){
        self.plog('INFO', a_msg, a_data);
    } else {
        self.plog('INFO', a_msg);
    }
}

module.exports.Logger = Logger;

