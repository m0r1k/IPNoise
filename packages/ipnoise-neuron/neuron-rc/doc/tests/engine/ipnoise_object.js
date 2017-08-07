var s_logger        = require('./logger');
var s_events        = require('events');
var s_extend        = require('extend');
var s_fs            = require('fs');
var s_util          = require('util');
var s_mongodb       = require('mongodb');
var s_assert        = require('assert');

var Logger          = s_logger.Logger;
var EventEmitter    = s_events.EventEmitter;

// ---------------- static ----------------

function isEmptyObject(a_obj)
{
    var ret = true;

    for (var key in a_obj){
        var res = Object.prototype.hasOwnProperty.call(
            a_obj,
            key
        );
        if (res){
            ret = false;
            break;
        }
    }

    return ret;
}

// ---------------- IPNoiseObject ----------------

function IPNoiseObject(a_args, a_props)
{
    var self = this;
    var args = {
        'trans':    undefined,
        'logger':   new Logger()
    };
    s_extend(args, a_args);

    if (!args.trans){
        args.logger.pfatal("missing argument: 'trans'\n", args);
    }

    self.m_is_new       = true;
    self.m_is_removed   = false;
    self.m_is_dirty     = isEmptyObject(a_props) ? false: true;

    self.m_trans        = args.trans;
    self.m_logger       = args.logger;

    self.m_props        = {
        '_id':          self.m_trans.createId(),
        'name':         '',
        'type':         'IPNoiseObject',
        'pos_x':        0,
        'pos_y':        0,
        'pos_z':        0,
        'parents':      {},
        'neighs_left':  {},
        'neighs_right': {},
        'children':     {}
    }
    s_extend(self.m_props, a_props);

    EventEmitter.call(self, a_args);

    self.on('actionRender',         self.forward);
    self.on('actionRenderHtml',     self.actionRenderHtml);
    self.on('actionPropsNamesGet',  self.actionPropsNamesGet);
    self.on('actionPropGet' ,       self.actionPropGet);
    self.on('actionPropSet',        self.actionPropSet);
    self.on('actionPosSet',         self.actionPosSet);
    self.on('actionObjectCreate',   self.actionObjectCreate);

    // add us to transaction
    self.m_trans.addObject(self);
}

s_util.inherits(IPNoiseObject, EventEmitter);

IPNoiseObject.prototype.getLogger = function()
{
    var self    = this;
    var logger  = self.m_logger;
    return logger;
}

IPNoiseObject.prototype.getId = function()
{
    var self = this;
    var id   = self.m_props._id;
    return id;
}

IPNoiseObject.prototype.setId = function(
    a_id)
{
    var self        = this;
    self.props._id  = a_id;
    self.setDirty(true);
}

IPNoiseObject.prototype.getName = function()
{
    var self = this;
    var name = self.m_props.name;
    return name;
}

IPNoiseObject.prototype.setName = function(
    a_name)
{
    var self = this;
    self.m_props.name = a_name;
    self.setDirty(true);
}

IPNoiseObject.prototype.isDirty = function()
{
    var self    = this;
    var dirty   = self.m_is_dirty;
    return dirty;
}

IPNoiseObject.prototype.setDirty = function(
    a_val)
{
    var self = this;

    self.m_is_dirty = a_val ? true : false;

    self.m_trans.addObject(self);
}

IPNoiseObject.prototype.getPos = function()
{
    var self = this;
    var pos  = {
        'x':    self.m_props.pos_x,
        'y':    self.m_props.pos_y,
        'z':    self.m_props.pos_z
    };
    return pos;
}

IPNoiseObject.prototype.setPos = function(
    a_val)
{
    var self    = this;
    var logger  = self.getLogger();

    if (a_val instanceof Array){
        self.m_props.pos_x = a_val[0];
        self.m_props.pos_y = a_val[1];
        self.m_props.pos_z = a_val[2];
    } else if (a_val instanceof Object){
        self.m_props.pos_x = a_val.x;
        self.m_props.pos_y = a_val.y;
        self.m_props.pos_z = a_val.z;
    } else {
        logger.pfatal("unsupported value", a_val);
    }

    setDirty(true);
}

IPNoiseObject.prototype.getTrans = function()
{
    var self    = this;
    var trans   = self.m_trans;
    return trans;
}

IPNoiseObject.prototype.setTrans = function(
    a_trans)
{
    var self     = this;
    self.m_trans = a_trans;
}

IPNoiseObject.prototype.isNew = function(
    a_val)
{
    var self = this;

    if (arguments.length){
        self.m_is_new = a_val;
    }

    return self.m_is_new;
}

IPNoiseObject.prototype.isRemoved = function()
{
    var self = this;
    return self.m_is_removed;
}

IPNoiseObject.prototype.setRemoved = function(
    a_val)
{
    var self = this;
    self.m_is_removed = a_val ? true : false;
}

IPNoiseObject.prototype.remove = function()
{
    var self = this;
    self.setRemoved(true);
}

IPNoiseObject.prototype.getProps = function()
{
    var self = this;
    return self.m_props;
}

IPNoiseObject.prototype.actionRenderHtml = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   undefined,
        'action':   undefined
    };

    s_extend(args, a_args);

    var res;
    var msg = {
        'to':       args.action.from,
        'from':     args.action.to
    };

    res = self.propReadSync({
        'name':     'dom_render',
        'is_json':  false
    });
    if (res){
        msg.data = res.data;
    }

    res = self.propReadSync({ 'name': 'pos' });
    if (res){
        msg.pos = res.obj;
    }

    args.socket.emit('actionRenderHtmlDone', msg);
}

IPNoiseObject.prototype.forward = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   undefined,
        'action':   undefined
    };
    s_extend(args, a_args);

    args.socket.emit('from_server', a_args.action);
}

IPNoiseObject.prototype.actionPropsNamesGet = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   undefined,
        'action':   undefined
    };

    var object_dir = self.getObjectDir();

    s_extend(args, a_args);

    var msg = {
        'to':       args.action.from,
        'from':     args.action.to,
        'type':     'actionPropsNamesGetDone',
        'data':     []
    };

    var inodes = s_fs.readdirSync(object_dir);
    for (var i = 0; i < inodes.length; i++){
        var inode   = inodes[i];
        var res     = s_fs.statSync(object_dir + inode);
        if (!res.isFile()){
            continue;
        }
        msg.data.push(inode);
    }

    args.socket.emit('from_server', msg);
}

IPNoiseObject.prototype.actionPropGet = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   undefined,
        'action':   undefined
    };

    s_extend(args, a_args);

    var msg = {
        'to':       args.action.from,
        'from':     args.action.to,
        'type':     'actionPropGetDone',
    };

    var prop_name = args.action.name;
    if (!prop_name){
        msg.error = "missing argument: 'name'";
        args.socket.emit('from_server', msg);
        return;
    }

    var is_json = true;
    if ('dom_render' == prop_name){
        is_json = false;
    }

    var res = self.propReadSync({
        'name':     prop_name,
        'is_json':  is_json
    });

    if (is_json){
        msg.data = res.obj;
    } else {
        msg.data = res.data;
    }

    args.socket.emit('from_server', msg);
}

IPNoiseObject.prototype.actionPropSet = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   undefined,
        'action':   undefined
    };

    var object_dir = self.getObjectDir();

    s_extend(args, a_args);

    var msg = {
        'to':       args.action.from,
        'from':     args.action.to,
        'type':     'actionPropSetDone'
    };

    var prop_name = args.action.name;
    if (!prop_name){
        msg.error = "missing argument: 'name'";
        args.socket.emit('from_server', msg);
        return;
    }

    var data    = args.action.data;
    var is_json = false;
    if ('object' == (typeof data)){
        is_json = true;
    }

    var cur_val_raw = self.propReadSync({
        'name':     prop_name,
        'is_json':  false
    });

    if (is_json){
        data = JSON.stringify(data);
    }

    if (cur_val_raw.data != data){
        self.propWriteSync({
            'name':     prop_name,
            'is_json':  is_json,
            'data':     args.action.data
        });
        var msg = {
            'to':       args.action.to,
            'from':     args.action.from,
            'type':     'actionPropChanged',
            'name':     prop_name,
            'data':     args.action.data
        };
        args.socket.broadcast.emit('from_server', msg);
    }

    args.socket.emit('from_server', msg);
}

IPNoiseObject.prototype.actionPosSet = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   a_args.socket,
        'action':   {
            'to':   a_args.action.to,
            'from': a_args.action.from,
            'type': 'actionPropSet',
            'name': 'pos',
            'data': {
                'x':    a_args.action.x,
                'y':    a_args.action.y,
                'z':    a_args.action.z
            }
        }
    };

    self.actionPropSet(args);
}

IPNoiseObject.prototype.actionObjectCreate = function(
    a_args)
{
    var self = this;
    var args = {
        'socket':   undefined,
        'action':   undefined
    };

    var object_dir = self.getObjectDir();

    s_extend(args, a_args);

    var object = new IPNoiseObject();
    var id     = object.getId();
    var dir    = object.getObjectDir();

    s_fs.mkdirSync(dir);

    object.propWriteSync({
        'is_json':  false,
        'name':     'dom_render',
        'data':     ""
            +"var widget = $('<div></div>')\n"
            +"    .appendTo(self.element)\n"
            +"    .ipnoise_widget(self.options);\n"
            +"\n"
            +"widget.html('<h1>hello from hell</h1>');\n"
            +"widget.css('width', '200px');\n"
            +"\n"
    });

    var msg = {
        'to':       args.action.from,
        'from':     id,
        'type':     'actionObjectCreateDone',
        'name':     args.action.name,
        'pos':      args.action.pos
    };

    args.socket.broadcast.emit('from_server', msg);

    return object;
}

/*
IPNoiseObject.prototype.save = function(
    a_cb)
{
    var self = this;

    var db         = self.getDb();
    var collection = db.collection('objects');

    console.log('save');

    if (self.isNew()){
        collection.insert(
            self.m_props,
            function(a_err, a_result){
                s_assert.equal(a_err, null);
                s_assert.equal(1, a_result.result.n);
                s_assert.equal(1, a_result.ops.length);
                console.log("document inserted\n");
                if (a_cb){
                    a_cb(a_result);
                }
            }
        );
    } else {
        collection.update(
            { '_id': self.m_props.id },
            { $set: self.m_props     },
            function(err, result){
                assert.equal(err, null);
                assert.equal(1, result.result.n);
                console.log("Updated the document with the field a equal to 2");
                if (a_cb){
                    a_cb(a_result);
                }
            }
        );
    }
}
*/

IPNoiseObject.prototype.createChild = function(
    a_link_name,
    a_props)
{
    var self    = this;
    var id      = self.getId();
    var trans   = self.getTrans();

    // obj will be added to transaction
    var obj = new IPNoiseObject(
        { 'trans': trans },
        a_props
    );

    self.m_props.children[a_link_name]  = obj.getId();
    obj.m_props.parents[id]             = id;

    trans.addObject(self);

    return obj;
}

module.exports.IPNoiseObject = IPNoiseObject;

