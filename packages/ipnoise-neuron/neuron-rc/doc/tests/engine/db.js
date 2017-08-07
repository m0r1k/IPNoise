var s_logger        = require('./logger');
var s_assert        = require('assert');
var s_mongodb       = require('mongodb');
var s_events        = require('events');
var s_extend        = require('extend');
var s_util          = require('util');
var s_transaction   = require('./transaction');

var Logger          = s_logger.Logger;
var OplogWatcher    = require('mongo-oplog-watcher');
var MongoClient     = s_mongodb.MongoClient;
var EventEmitter    = s_events.EventEmitter;
var ObjectID        = s_mongodb.ObjectID;
var Transaction     = s_transaction.Transaction;

function Db(a_args)
{
    var self = this;
    var args = {
        'url':      undefined,
        'logger':   new Logger({ 'prefix': 'DB: ' })
    };
    s_extend(args, a_args);

    self.m_url      = args.url;
    self.m_conn     = undefined;
    self.m_logger   = args.logger;
    self.m_trans0   = new Transaction({ 'db': self });

    EventEmitter.call(self, a_args);

    self.on('actionConnect', self.actionConnect);
}

s_util.inherits(Db, EventEmitter);

Db.prototype.getLogger = function()
{
    var self    = this;
    var logger  = self.m_logger;
    return logger;
}

Db.prototype.getTrans0 = function()
{
    var self = this;
    return self.m_trans0;
}

Db.prototype.createId = function()
{
    var id = new ObjectID().toString();
    return id;
}

Db.prototype.actionConnect = function(a_args)
{
    var self    = this;
    var logger  = self.getLogger();

    var args = {
        'url':  self.m_url
    };
    s_extend(args, a_args);

    if (self.m_conn){
        self.m_conn.close();
        self.m_conn = undefined;
    }

    if (a_args.url){
        self.m_url = a_args.url;
    }

    if (!self.m_url){
        logger.pfatal("unknown 'url' for connect");
    }

    logger.pinfo("connect to: '"+self.m_url+"'");

    MongoClient.connect(self.m_url, function(a_err, a_conn){
        self.connectedCb(a_err, a_conn);
    });
}

Db.prototype.startOplog = function()
{
    var self    = this;
    var logger  = self.getLogger();

    // mongo replication read
    self.oplog = new OplogWatcher({
        'host':   'felizcarros.com' ,
        'ns':     'network.objects'
    });

    self.oplog.on('insert', function(a_doc){
        self.dbInsertCb(a_doc);
    });
    self.oplog.on('update', function(a_doc){
        self.dbUpdateCb(a_doc);
    });
    self.oplog.on('delete', function(a_doc){
        self.dbDeleteCb(a_doc);
    });

    logger.pinfo('op log started');
}

Db.prototype.getCollection = function(
    a_args)
{
    var self        = this;
    var logger      = self.getLogger();
    var args        = {
        'name': 'objects'
    };
    s_extend(args, a_args);

    var collection = self.m_conn.collection(args.name);

    return collection;
}

Db.prototype.connectedCb = function(a_err, a_conn)
{
    var self    = this;
    var logger  = self.getLogger();

    logger.pinfo("connected to: '"+self.m_url+"'");

    if (self.m_conn){
        self.m_conn.close();
        self.m_conn = undefined;
    }
    self.m_conn = a_conn;

    // start oplog
    self.startOplog();

    self.emit('actionConnectDone', a_err, self);
}

Db.prototype.dbInsertCb = function(
    a_doc)
{
    var self    = this;
    var logger  = self.getLogger();
    var trans0  = self.getTrans0();
    var obj     = trans0.docToObject(a_doc);

    logger.pinfo('event insert', a_doc);

    trans0.memAddToGeneric(obj);
}

Db.prototype.dbUpdateCb = function(
    a_doc)
{
    var self    = this;
    var logger  = self.getLogger();
    var trans0  = self.getTrans0();
    var obj     = trans0.docToObject(a_doc);

    logger.pinfo('event insert', a_doc);

    trans0.memAddToGeneric(obj);

    logger.pinfo('event update', a_doc);
}

Db.prototype.dbDeleteCb = function(
    a_id)
{
    var self    = this;
    var logger  = self.getLogger();
    var trans0  = self.getTrans0();

    trans0.memRemoveFromAll(a_id);

    logger.pinfo('event delete', a_doc);
}

Db.prototype.load = function(
    a_args,
    a_cb,
    a_cb_args)
{
    var self        = this;
    var logger      = self.getLogger();
    var collection  = self.getCollection();

    var args = {};
    s_extend(args, a_args);

    if ('string' == (typeof args._id)){
        args._id = new ObjectID(args._id);
    }

    logger.pinfo('search documents: ', args);

    collection.find(args).toArray(function(a_err, a_docs){
        if (a_err){
            logger.pfatal("cannot load documents", {
                'a_err':    a_err,
                'a_docs':   a_docs,
                'args':     args
            });
        }

        self.loadCb(a_err, a_docs, {
            'cb':       a_cb,
            'cb_args':  a_cb_args
        });
    });
}

Db.prototype.loadCb = function(
    a_err,
    a_docs,
    a_args)
{
    var self = this;

    if (a_args.cb){
        a_args.cb(a_err, a_docs, a_args.cb_args);
    }
}

Db.prototype.loadOne = function(
    a_args,
    a_cb,
    a_cb_args)
{
    var self = this;

    self.load(a_args, self.loadOneCb, {
        'cb':         a_cb,
        'cb_args':    a_cb_args
    });
}

Db.prototype.loadOneCb = function(
    a_err,
    a_docs,
    a_args)
{
    var self = this;

    if (a_args.cb){
        var doc = a_docs[0];
        a_args.cb(a_err, doc, a_args.cb_args);
    }
}

Db.prototype.insert = function(
    a_docs,
    a_cb,
    a_cb_args)
{
    var self        = this;
    var logger      = self.getLogger();
    var collection  = self.getCollection();

    if (!a_docs.length){
        logger.pwarn("empty documents");
    }

    collection.insert(
        a_docs,
        function(a_err, a_result){
            //s_assert.equal(a_err, null);
            //s_assert.equal(1, a_result.result.n);
            //s_assert.equal(1, a_result.ops.length);
            // console.log("document inserted\n");
            //if (a_cb){
            //    a_cb(a_result);
            //}
        }
    );
}

Db.prototype.update = function(
    a_docs,
    a_cb,
    a_cb_args)
{
    var self        = this;
    var logger      = self.getLogger();
    var collection  = self.getCollection();

    if (!a_docs.length){
        logger.pwarn("empty documents");
    }

    for (var i = 0; i < a_docs.length; i++){
        var doc = a_docs[i];
        collection.update(
            { '_id': new ObjectID(doc._id) },
            { $set: doc },
            function(a_err, a_result){
                //s_assert.equal(a_err, null);
                //s_assert.equal(1, a_result.result.n);
                //s_assert.equal(1, a_result.ops.length);
                //console.log("document updated\n");
                //if (a_cb){
                //    a_cb(a_result);
                //}
            }
        );
    }
}

Db.prototype.remove = function(
    a_docs,
    a_cb,
    a_cb_args)
{
    var self        = this;
    var logger      = self.getLogger();
    var collection  = self.getCollection();

    if (!a_docs.length){
        logger.pwarn("empty documents");
    }

    for (var i = 0; i < a_docs.length; i++){
        var doc = a_docs[i];
        collection.remove(
            doc,
            function(a_err, a_result){
                //s_assert.equal(a_err, null);
                //s_assert.equal(1, a_result.result.n);
                //s_assert.equal(1, a_result.ops.length);
                //console.log("document removed\n");
                //if (a_cb){
                //    a_cb(a_result);
                //}
            }
        );
    }
}

module.exports.Db = Db;

