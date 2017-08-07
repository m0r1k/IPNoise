var g_logger            = require('./logger');
var g_ipnoise_object    = require('./ipnoise_object');
var IPNoiseObject       = g_ipnoise_object.IPNoiseObject;
var g_extend            = require('extend');
var g_events            = require('events');
var g_fs                = require('fs');
var g_util              = require('util');

// ---------------- schema and model ----------------

// ---------------- IPNoiseObjectViewer ----------------

function IPNoiseObjectViewer(a_args, a_props)
{
    var self = this;

    var args = {
        'trans':        undefined,
        'm_socket':     undefined,
        'm_server':     undefined
    };
    g_extend(args, a_args);

    var props = {
        'type':         'IPNoiseObjectViewer',
        'min_x':        0,
        'max_x':        1000,
        'min_y':        0,
        'max_y':        1000,
        'min_z':        0,
        'max_z':        1000
    };
    g_extend(props, a_props);

    IPNoiseObject.call(self, args, props);

    self.m_socket   = args.socket;
    self.m_server   = args.server;
    self.m_only_id  = args.only_id;

    self.on('render', self.render);
}

g_util.inherits(IPNoiseObjectViewer, IPNoiseObject);

IPNoiseObjectViewer.prototype.getZone = function()
{
    var self = this;
    var zone = {
        'min_x':    self.m_props.min_x,
        'max_x':    self.m_props.max_x,
        'min_y':    self.m_props.min_y,
        'max_y':    self.m_props.max_y,
        'min_z':    self.m_props.min_z,
        'max_z':    self.m_props.max_x
    };
    return zone;
}

IPNoiseObjectViewer.prototype.render = function()
{
    var self    = this;
    var logger  = self.getLogger();
    var trans   = self.getTrans();
    var id      = self.getId();
    var db      = trans.getDb();

    logger.pinfo("search objects for viewer");

    db.load(
        {
            'pos_x':    {
                '$lte': self.m_props.max_x,
                '$gte': self.m_props.min_x
            },
            'pos_y':    {
                '$lte': self.m_props.max_y,
                '$gte': self.m_props.min_y
            },
            'pos_z':    {
                '$lte': self.m_props.max_z,
                '$gte': self.m_props.min_z
            }
        },
        function(a_err, a_docs, a_args){
            logger.pinfo("found: a_docs", a_docs);

            var action = {
                'to':       id,
                'data':     a_docs
            };

            self.m_socket.emit('actionAddForDraw', action);
        }
    );

/*
    var db_root_dir = self.m_server.getDbRootDir();
    var id          = self.getId();

    var msg = {
        'to':       id,
        'data':     []
    };

    if (self.m_only_id){
        msg.data.push(self.only_id);
    } else {
        var inodes = g_fs.readdirSync(db_root_dir);
        for (var i = 0; i < inodes.length; i++){
            var inode   = inodes[i];
            var res     = g_fs.statSync(db_root_dir + inode);
            if (!res.isDirectory()){
                continue;
            }
            msg.data.push(inode);
        }
    }

    self.m_socket.emit('actionAddForDraw', msg);
*/
}

module.exports.IPNoiseObjectViewer  = IPNoiseObjectViewer;
//module.exports.schema_info          = schema_info;
//module.exports.schema               = schema;

