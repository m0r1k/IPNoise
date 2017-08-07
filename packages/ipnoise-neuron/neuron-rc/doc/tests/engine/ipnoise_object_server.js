var s_logger                = require('./logger');
var s_ipnoise_object        = require('./ipnoise_object');
var s_ipnoise_object_viewer = require('./ipnoise_object_viewer');
//var s_transaction           = require('./transaction');
var s_extend                = require('extend');
var s_events                = require('events');
var s_fs                    = require('fs');
var s_util                  = require('util');
var s_assert                = require('assert');
var s_app                   = require('express')();
var s_http                  = require('http');
var s_http_server           = s_http.Server(s_app);
var s_io                    = require('socket.io')(s_http_server);

var Logger              = s_logger.Logger;
var IPNoiseObject       = s_ipnoise_object.IPNoiseObject;
var IPNoiseObjectViewer = s_ipnoise_object_viewer.IPNoiseObjectViewer;

// ---------------- IPNoiseObjectServer ----------------

function IPNoiseObjectServer(a_args, a_props)
{
    var self    = this;
    var args    = {
        'port':     8080,
        'trans':    undefined,
        'logger':   new Logger({ 'prefix': 'Server: ' })
    };
    s_extend(args, a_args);

    var props   = {
        'type':     'IPNoiseObjectServer'
    }
    s_extend(props, a_props);

    if (!args.trans){
        args.logger.pfatal("missing argument: 'trans'", args);
    }

    IPNoiseObject.call(self, args, props);

    self.m_port     = args.port;
    self.m_viewers  = {};
    self.m_cache    = {};
    self.m_logger   = args.logger;

    // events handler
    self.on('actionStart',  self.actionStart);
}

s_util.inherits(IPNoiseObjectServer, IPNoiseObject);

IPNoiseObjectServer.prototype.getPort = function()
{
    var self = this;
    var port = self.m_port;
    return port;
}

IPNoiseObjectServer.prototype.setPort = function(
    a_val)
{
    var self = this;
    self.m_port = a_val;
}

IPNoiseObjectServer.prototype.actionStart = function(
    a_args)
{
    var self    = this;
    var logger  = self.getLogger();

    if (undefined != a_args.port){
        self.setPort(a_args.port);
    }

    var port = self.getPort();

    self.session = s_io
        .of('/session')
        .on('connection', function(a_socket){
            self.sessionConnectionOpen(a_socket);
        });

    s_http_server.listen(port, function(){
        var err = undefined;
        logger.pinfo("Server listening on: *:" + port);
        self.emit('actionStartDone', err, self);
    });
}

IPNoiseObjectServer.prototype.sessionConnectionOpen = function(
    a_socket)
{
    var self    = this;
    var logger  = self.getLogger();

    logger.pinfo('session socket opened');

    //  socket.broadcast.emit(
    //      'from_server',
    //      { text: 'new user connected' }
    //  );

    a_socket.on('actionObjectCreate', function(a_action){
        logger.pinfo('request object create');
        self.actionObjectCreate(a_socket, a_action);
    });

    a_socket.on('actionViewerCreate', function(a_action){
        logger.pinfo('request open session');
        self.actionViewerCreate(a_socket, a_action);
    });

    a_socket.on('actionRenderHtml', function(a_action){
        logger.pinfo('request render html');
        self.processAction('actionRenderHtml',
            a_socket,
            a_action
        );
    });

/*
    a_socket.on('to_server', function(a_action){
        var type    = a_action.type;
        var from    = a_action.from;
        var to      = a_action.to;

        console.log("new action:"
            +" from: '" +   from   +"',"
            +" to: '"   +   to     +"',"
            +" type: '" +   type   +"'"
        );

        self.processAction(
            a_socket,
            a_action
        );
    });
*/

    a_socket.on('disconnect', function(a_socket){
        self.sessionConnectionClosed(a_socket);
    });
}

IPNoiseObjectServer.prototype.sessionConnectionClosed = function(
    a_socket)
{
    var self    = this;
    var logger  = self.getLogger();

    logger.pinfo('session socket closed');
}

IPNoiseObjectServer.prototype.actionObjectCreate = function(
    a_socket,
    a_action)
{
    var self = this;

    var args = {};

    s_extend(args, a_action);

    args.socket = a_socket;
    args.server = self;

    var object  = new IPNoiseObject(args);
    var id      = object.getId();
    var pos     = object.getPos();

    var action  = {
        'object':   {
            'id':   id,
            'type': 'ipnoise_object'
        }
    };

    a_socket.emit('actionObjectCreateDone', action);

    // add to viewers
    for (var socket_id in self.m_viewers){
        var viewer = self.m_viewers[socket_id];
        var zone   = viewer.getZone();

        if (    pos.x >= zone.min_x
            &&  pos.x <= zone.max_x
            &&  pos.y >= zone.min_y
            &&  pos.y <= zone.max_y
            &&  pos.z >= zone.min_z
            &&  pos.z <= zone.max_z)
        {
            viewer.addForDraw(object);

            self.socket.emit('actionAddForDraw', msg);
        }
    }
}

IPNoiseObjectServer.prototype.actionViewerCreate = function(
    a_socket,
    a_action)
{
    var self    = this;
    var logger  = self.getLogger();
    var trans   = self.getTrans().createTransaction();

    var args    = {};
    s_extend(args, a_action);

    args.logger = logger;
    args.socket = a_socket;
    args.trans  = trans;
    args.server = self;

    var props   = {
        //'_id':  '557c491eada9545221db419a' // TODO XXX FIXME remove me
    }

    var viewer  = new IPNoiseObjectViewer(args, props);
    var id      = viewer.getId();

    logger.pinfo('request create viewer! ID: '+id);

    self.m_viewers[a_socket.id] = viewer;

    trans.commit(function(){
        console.log('trans commited');
        var action = {
            'object':   {
                'id':   id,
                'type': 'ipnoise_object_viewer'
            }
        };

        a_socket.emit('actionViewerCreateDone', action);

        viewer.emit('render');
    });
}

IPNoiseObjectServer.prototype.processAction = function(
    a_action_type,
    a_socket,
    a_action)
{
    var self = this;

    var res;
    var action = {
        'from':         undefined,
        'from_object':  undefined,
        'to':           undefined,
        'to_object':    undefined,
        'type':         undefined
    };

    s_extend(action, a_action);

    var msg = {
        'to':       action.from,
        'from':     action.to,
        'type':     action.type
    };

    // check from
    res = self.getObject(action.from);
    if (res.error){
        msg.error = res.error;
        a_socket.emit('from_server', msg);
        return;
    }
    action.from_object = res.object;

    // check to
    res = self.getObject(action.to);
    if (res.error){
        msg.error = res.error;
        a_socket.emit('from_server', msg);
        return;
    }
    action.to_object = res.object;

    // emit action
    action.to_object.emit(a_action_type, {
        'socket':   a_socket,
        'action':   action
    });
}

IPNoiseObjectServer.prototype.createTransaction = function()
{
    var self    = this;
    var trans   = self.m_trans.createTransaction();

    return trans;
}

module.exports.IPNoiseObjectServer = IPNoiseObjectServer;

