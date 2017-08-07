var s_logger                = require('./logger');
var s_util                  = require('util');
var s_extend                = require('extend');
var s_assert                = require('assert');
var s_events                = require('events');
var s_ipnoise_object        = require('./ipnoise_object');
var s_ipnoise_object_server = require('./ipnoise_object_server');
var s_ipnoise_object_viewer = require('./ipnoise_object_viewer');

var Logger              = s_logger.Logger;
var EventEmitter        = s_events.EventEmitter;
var IPNoiseObject       = s_ipnoise_object.IPNoiseObject;
var IPNoiseObjectServer = s_ipnoise_object_server.IPNoiseObjectServer;
var IPNoiseObjectViewer = s_ipnoise_object_viewer.IPNoiseObjectViewer;

console.log(s_ipnoise_object_server);

function Transaction(a_args)
{
    var self = this;
    var args = {
        'parent':   undefined,
        'db':       undefined,
        'logger':   new Logger({ 'prefix': 'Transaction: ' })
    };
    s_extend(args, a_args);

    self.m_db               = args.db;
    self.m_parent           = args.parent;
    self.m_objects_generic  = {};
    self.m_objects_dirty    = {};
    self.m_logger           = args.logger;

    EventEmitter.call(self, a_args);
}

s_util.inherits(Transaction, EventEmitter);

Transaction.prototype.getLogger = function()
{
    var self    = this;
    var logger  = self.m_logger;
    return logger;
}

Transaction.prototype.getDb = function()
{
    var self = this;
    var db   = self.m_db;
    return db;
}

Transaction.prototype.createId = function()
{
    var self = this;
    var db   = self.getDb();
    var id   = db.createId();
    return id;
}

Transaction.prototype.memRemoveFromGeneric = function(
    a_id)
{
    var self    = this;
    var logger  = self.getLogger();
    var type    = typeof a_id;

    if ('string' != type){
        logger.pfatal("type of argument: 'a_id' is not string "
            +" (" + type + ")"
        );
    }

    delete self.m_objects_generic[a_id];
}

Transaction.prototype.memRemoveFromDirty = function(
    a_id)
{
    var self    = this;
    var logger  = self.getLogger();
    var type    = typeof a_id;

    if ('string' != type){
        logger.pfatal("type of argument: 'a_id' is not string "
            +" (" + type + ")"
        );
    }

    delete self.m_objects_dirty[a_id];
}

Transaction.prototype.memRemoveFromAll = function(
    a_id)
{
    var self    = this;
    var logger  = self.getLogger();
    var type    = typeof a_id;

    if ('string' != type){
        logger.pfatal("type of argument: 'a_id' is not string "
            +" (" + type + ")"
        );
    }

    self.memRemoveFromDirty(a_id);
    self.memRemoveFromGeneric(a_id);
}

Transaction.prototype.memAddToGeneric = function(
    a_object)
{
    var self      = this;
    var logger    = self.getLogger();
    var object_id = undefined;

    if (!(a_object instanceof IPNoiseObject)){
        logger.pfatal("missing argument a_object"
            +" or it's not instanceof IPNoiseObject"
        );
    }

    object_id = a_object.getId();
    self.m_objects_generic[object_id] = a_object;
}

Transaction.prototype.memAddToDirty = function(
    a_object)
{
    var self      = this;
    var logger    = self.getLogger();
    var object_id = undefined;

    if (!(a_object instanceof IPNoiseObject)){
        logger.pfatal("missing argument a_object"
            +" or it's not instanceof IPNoiseObject"
        );
    }

    object_id = a_object.getId();
    self.m_objects_dirty[object_id] = a_object;
}

Transaction.prototype.addObject = function(
    a_object)
{
    var self = this;

    s_assert.equal('object', typeof a_object);

    var object_id   = a_object.getId();
    var is_dirty    = a_object.isDirty();

    if (is_dirty){
        self.memRemoveFromGeneric(object_id);
        self.memAddToDirty(a_object);
    } else {
        self.memRemoveFromDirty(object_id);
        self.memAddToGeneric(a_object);
    }
}

Transaction.prototype.getObjectById = function(
    a_id,
    a_cb,
    a_cb_args)
{
    var self    = this;
    var logger  = self.getLogger();
    var obj     = undefined;

    if (!a_id){
        logger.pfatal("missing argument: 'id'",
            arguments
        );
    }

    logger.pinfo("getObjectById: '"+a_id+"'");

    do {
        // search through dirty
        obj = self.m_objects_dirty[a_id];
        if (obj){
            logger.pinfo('    found in m_objects_dirty');
            if (a_cb){
                a_cb(undefined, obj, a_cb_args);
            }
            break;
        }

        // search through generic
        obj = self.m_objects_generic[a_id];
        if (obj){
            logger.pinfo('    found in m_objects_generic');
            if (a_cb){
                a_cb(undefined, obj, a_cb_args);
            }
            break;
        }

        // load from db
        logger.pinfo('    search in db');
        self.m_db.loadOne(
            { '_id': a_id },
            function (a_err, a_doc, a_args){
                self.loadOneCb(a_err, a_doc, a_args);
            },
            {
                'cb':       a_cb,
                'cb_args':  a_cb_args
            }
        );
    } while (0);
}

Transaction.prototype.docToObject = function(
    a_doc)
{
    var self    = this;
    var logger  = self.getLogger();

    if (!a_doc){
        logger.pfatal("missing argument: 'a_doc'");
    }

    var obj     = undefined;
    var type    = a_doc.type;

    if ('IPNoiseObject' == type){
        obj = new IPNoiseObject(
            { 'trans': self },
            a_doc
        );
    } else if ('IPNoiseObjectViewer' == type){
        obj = new IPNoiseObjectViewer(
            { 'trans': self },
            a_doc
        );
    } else if ('IPNoiseObjectServer' == type){
        obj = new IPNoiseObjectServer(
            { 'trans': self },
            a_doc
        );
    } else {
        logger.pfatal('unsupported object type', {
            'doc':  a_doc,
            'type': type
        });
    }

    obj.setDirty(false);
    obj.isNew(false);

    return obj;
}

Transaction.prototype.loadOneCb = function(
    a_err, a_doc, a_args)
{
    var self    = this;
    var objects = [];
    var logger  = self.getLogger();
    var obj     = undefined;

    if (a_doc){
        obj = self.docToObject(a_doc);
    }

    if (a_args.cb){
        a_args.cb(a_err, obj, a_args.cb_args);
    }
}

Transaction.prototype.dump = function(
    a_msg)
{
    var self    = this;
    var logger  = self.getLogger();

    logger.pinfo(a_msg);
    logger.pinfo('dump transaction');

    logger.pinfo('    dump objects normal:');
    for (var id in self.m_objects_generic){
        var object = self.m_objects_generic[id];
        logger.pinfo('        '+object.getId());
    }

    logger.pinfo('    dump objects dirty:');
    for (var id in self.m_objects_dirty){
        var object = self.m_objects_dirty[id];
        logger.pinfo('        '+object.getId());
    }
}

Transaction.prototype.commit = function(
    a_cb,
    a_cb_args)
{
    var self    = this;
    var logger  = self.getLogger();
    var db      = self.getDb();

    var docs_for_insert = [];
    var docs_for_update = [];
    var docs_for_remove = [];

    for (var id in self.m_objects_dirty){
        var object       = self.m_objects_dirty[id];
        var object_props = object.getProps();

        if (object.isRemoved()){
            docs_for_remove.push(object_props);
        } else if (object.isNew()){
            docs_for_insert.push(object_props);
        } else {
            docs_for_update.push(object_props);
        }
    }

    // remove all from dirty list
    self.m_objects_dirty = {};

    //logger.pinfo('docs for commit: ', {
    //    'docs_for_insert':  docs_for_insert,
    //    'docs_for_update':  docs_for_update,
    //    'docs_for_remove':  docs_for_remove
    //});

    // insert
    if (docs_for_insert.length){
        db.insert(docs_for_insert);
    }

    // update
    if (docs_for_update.length){
        db.update(docs_for_update);
    }

    // remove
    if (docs_for_remove.length){
        db.remove(docs_for_remove);
    }
}

Transaction.prototype.createTransaction = function()
{
    var self = this;

    var trans = new Transaction({
        'parent':   self,
        'db':       self.m_db
    });

    return trans;
}

module.exports.Transaction = Transaction;

