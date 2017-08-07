var s_logger                = require('./logger');
var s_ipnoise_object        = require('./ipnoise_object');
var s_ipnoise_object_server = require('./ipnoise_object_server');
var s_db                    = require('./db');

var Logger              = s_logger.Logger;
var IPNoiseObject       = s_ipnoise_object.IPNoiseObject;
var IPNoiseObjectServer = s_ipnoise_object_server.IPNoiseObjectServer;
var Db                  = s_db.Db;

var logger  = new Logger({ 'prefix': 'App: ' });
var db      = new Db();
var server  = undefined;

function test_trans()
{
    var trans0  = db.getTrans0();

    var obj1    = new IPNoiseObject({ 'trans': trans0 });
    var obj1_id = obj1.getId();

    var obj2    = new IPNoiseObject({ 'trans': trans0 });
    var obj2_id = obj2.getId();

    obj1.setName('test');

    var trans1 = trans0.createTransaction();
    var obj3    = new IPNoiseObject({ 'trans': trans1 });
    var obj3_id = obj3.getId();

    logger.pinfo('obj3_id: '+obj3_id);

    trans1.getObjectById(
        obj3_id,
        function(a_err, a_object, a_args){
            if (a_object){
                logger.pinfo('object loaded: ' + a_object.getId());

                var d = new Date();
                var n = d.getDate();
                a_object.setName('tigra was here '+ n);
            }

            var obj4 = new IPNoiseObject({ 'trans': trans1 });
            obj4.setName('show must go on');

            //a_object.remove();

            //trans0.dump('trans0');
            //trans1.dump('trans1');
            trans1.commit();
        }
    );
}

db.on('actionConnectDone', function(a_err, a_db){
    if (a_err){
        logger.pfatal("cannot init db\n", a_err);
    }

    logger.pinfo('DB inited');

    var trans0 = db.getTrans0();

    server = new IPNoiseObjectServer(
        { 'trans': trans0 },
        {
            '_id':  'server',
            'name': 'server'
        }
    );

    trans0.commit();

    server.on('actionStartDone', function(a_err, a_server){
        logger.pinfo("server started (ID: '"+server.getId()+"')");

        //test_trans();
    });

    server.emit('actionStart', {
        'port': 8106
    });
});

db.emit('actionConnect', {
    'url':  'mongodb://felizcarros.com:27017/network'
});

