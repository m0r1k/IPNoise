// ---------------- static ----------------

// ---------------- ObjectBrowserKeyboard ----------------

function ObjectBrowserKeyboard()
{
    var self = this;
}

ObjectBrowserKeyboard.prototype                  = new Object;
ObjectBrowserKeyboard.prototype.getObjectType    = function()
{
    return 'core.object.browser.keyboard';
}

ObjectBrowserKeyboard.prototype.v_reset = function(
    a_args)
{
    var self    = this;
    var args    = {
        'document': undefined
    };
    copy_args(args, a_args);

    if (!args.document){
        self.getLogger().pfatal(
            "missing argument: 'document'"
        );
    }

    // init super
    Object.prototype.v_reset.apply(self, [ args ]);

    var doc = $(args.document);

    // init us
    self.m_cur_pressed_keys = {};

    doc.keyup(  function (a_ev){self.handleKeyUp(a_ev);});
    doc.keydown(function (a_ev){self.handleKeyDown(a_ev);});
}

ObjectBrowserKeyboard.prototype.handleKeyDown = function(
    a_ev)
{
    var self = this;
    var code = a_ev.keyCode;

    self.getLogger().pdebug(10,
        "key down, code: '"+code+"'"
    );

    self.setCurrentlyPressedKey({
        'code':     code,
        'pressed':  true
    });
}

ObjectBrowserKeyboard.prototype.handleKeyUp = function(
    a_ev)
{
    var self = this;
    var code = a_ev.keyCode;

    var data = [];

    data.append(1);

    console.log(data);

    //self.addChunkType(CHUNK_TYPE_ACTION_KEYBOARD_KEY_UP)
    //self.addChunkType(CHUNK_TYPE_ACTION_KEYBOARD_KEY_UP)

    //self.getLogger().pdebug(10,
    //    "key up, code: '"+code+"'"
    //);

    // self.setCurrentlyPressedKey({
    //    'code':     code,
    //    'pressed':  false
    //});
}

ObjectBrowserKeyboard.prototype.getCurrentlyPressedKeys = function()
{
    var self = this;
    return self.m_cur_pressed_keys;
}

ObjectBrowserKeyboard.prototype.initKeyInfo = function(
    a_code)
{
    var self = this;

    if (!self.m_cur_pressed_keys[a_code]){
        self.m_cur_pressed_keys[a_code] = {}
    }
    self.m_cur_pressed_keys[a_code].time_press    = 0;
    self.m_cur_pressed_keys[a_code].time_elapsed  = 0;
    // all keys in 'inc' will be incremented
    self.m_cur_pressed_keys[a_code].inc           = {};

    return self.m_cur_pressed_keys[a_code];
}

ObjectBrowserKeyboard.prototype.setCurrentlyPressedKey = function(
    a_args)
{
    var self    = this;
    var code    = a_args.code;
    var pressed = a_args.pressed;

    var cur_time_ms = new Date().getTime();

    var key_info = self.m_cur_pressed_keys[code];
    if (!key_info){
        key_info = self.initKeyInfo(code);
    }

    if (pressed){
        if (!key_info.time_press){
            // first press, mark start time
            key_info.time_press      = cur_time_ms;
            key_info.time_elapsed    = 0;
            key_info.time_last_check = cur_time_ms;
        } else {
            // cont press
            var delta  = cur_time_ms - key_info.time_last_check;
            key_info.time_elapsed += delta;
            // update handlers variable
            for (var key in key_info.inc){
                key_info.inc[key] += delta;
            }
            key_info.time_last_check = cur_time_ms;
        }
    } else {
        delete self.m_cur_pressed_keys[code];
    }
}

// ---------------- module_info ----------------

var module_info = {
    'type':         ObjectBrowserKeyboard.prototype.getObjectType(),
    'objectCreate': function(){
        return new ObjectBrowserKeyboard();
    }
};
module_register(module_info);

