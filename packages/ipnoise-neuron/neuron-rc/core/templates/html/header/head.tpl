function logout(object_id, args){
    var packet = {
        'object_id':  object_id,
        'method':     'logout'
    };
    var args = {
        'packet':   packet,     
        'success':  function(data){
            var params = data.params;
            if (params && params.redirect){
                gotoObject(params.redirect);
            }
        }
    };
    sendApiPacket(args);
}

function login(object_id, args){
    var packet = {
        'object_id':  object_id,
        'method':     'login',
        'params':     {
            'login':        args.login,
            'password':     args.password
        }
    };
    var args = {
        'packet':   packet,     
        'success':  function(data){
            var params = data.params;
            if (params && params.redirect){
                gotoObject(params.redirect);
            }
        }
    };
    sendApiPacket(args);
}

function register(object_id, args){
    var packet = {
        'object_id':  object_id,
        'method':     'register',
        'params':     {
            'login':        args.login,
            'password1':    args.password1,
            'password2':    args.password2
        }
    };
    var args = {
        'packet':   packet,     
        'success':  function(data){
            var params = data.params;
            if (params && params.redirect){
                gotoObject(params.redirect);
            }
        }
    };
    sendApiPacket(args);
}

$(document).ready(function(){
    $('#login_tabs').tabs();
    $('#login').submit(function(){
        login(
            getCurObjectId(),
            {
                'login':    $('#login').find('input[name="login"]').val(),
                'password': $('#login').find('input[name="password"]').val()
            }
        );
        return false;
    });
    $('#register').submit(function(){
        register(
            getCurObjectId(),
            {
                'login':        $('#register').find('input[name="login"]').val(),
                'password1':    $('#register').find('input[name="password1"]').val(),
                'password2':    $('#register').find('input[name="password2"]').val()
            }
        );
        return false;
    });
    $('div#login_dialog').dialog({
        'autoOpen':     false,
        'modal':        true,
        'title':        'Добро пожаловать!'
    });
    $('#login_dialog').click(function(){
        $('div#login_dialog').dialog('open');
    });
    $('#logout_dialog').click(function(){
        var res = confirm(
            "Are you sure want quit?",
            0
        );
        if (!res){
            return;
        }
        logout(getCurObjectId());
        return false;
    });
});

