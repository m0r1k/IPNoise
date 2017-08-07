function createChunk(a_name){
    var obj;

    eval("obj = new "+a_name+"()");
    if (!obj){
        alert(
            "cannot create object: '"+a_name+"'"
        );
    }

    return obj;
}

function clearAfter(a_el)
{
    var cur = a_el.next();

    while (cur.length){
        var tmp = cur;
        cur = cur.next();
        tmp.remove();
    }
}

// ---------------- Chunk ----------------

function Chunk(){
    var self = this;
}

Chunk.prototype.getType = function()
{
    return 'Chunk';
}
Chunk.prototype.createUi = function()
{
    var self    = this;
    var div     = $('<div class="line"/>');
    var el      = undefined;

    el = self.i_createUi();
    if (el){
        div.append(el);
    }

    return div;
}

// ---------------- ChunkNop ----------------
function ChunkNop()
{
    var self = this;
}
ChunkNop.prototype = new Chunk;
ChunkNop.prototype.getType = function()
{
    return 'ChunkNop';
}
ChunkNop.prototype.i_createUi = function()
{
    var self        = this;
    var div         = $('<div class="line"/>');
    var select      = $('<select/>');
    var vals_info   = [
        {   'title': '' },
        {
            'title': 'if',
            'class': 'ChunkIf'
        },
        {
            'title': '{',
            'class': 'ChunkOpen'
        },
        {
            'title': '}',
            'class': 'ChunkClose'
        },
        {
            'title': 'select object',
            'class': 'ChunkObjectSelect'
        },
        {
            'title': 'check property',
            'class': 'ChunkCheckProp'
        }
    ];

    select.change(function(a_ev){
        var target = $(a_ev.target);
        var parent = target.parent();
        var val    = target.val();
        var obj    = createChunk(val);
        var el     = obj.createUi();
        var cur    = target.next();

        while (cur.length){
            var tmp = cur;
            cur = cur.next();
            tmp.remove();
        }

        parent.append(el);
    });

    for (var i = 0; i < vals_info.length; i++){
        var val_info = vals_info[i];
        var option   = $('<option>'+val_info.title+'</option>');
        option.attr("value", val_info.class);
        select.append(option);
    }
    div.append(select);

    return div;
}

// ---------------- ChunkObjectSelect ----------------
function ChunkObjectSelect()
{
    var self = this;
}
ChunkObjectSelect.prototype = new Chunk;
ChunkObjectSelect.prototype.getType = function()
{
    return 'ChunkObjectSelect';
}
ChunkObjectSelect.prototype.i_createUi = function()
{
    var self        = this;
    var div         = $('<div/>');
    var select      = $('<select/>');
    var vals_info   = [
        { 'title': ''           },
        { 'title': 'action'     },
        { 'title': 'self'       }
    ];

    select.change(function(a_ev){
        var target = $(a_ev.target);
        clearAfter(target);
    });

    for (var i = 0; i < vals_info.length; i++){
        var val_info = vals_info[i];
        var option   = $('<option>'+val_info.title+'</option>');
        select.append(option);
    }
    div.append(select);

    return div;
}

// ---------------- ChunkIf ----------------
function ChunkIf()
{
    var self = this;
}
ChunkOpen.prototype = new Chunk;
ChunkOpen.prototype.getType = function()
{
    return 'ChunkIf';
}
ChunkOpen.prototype.i_createUi = function()
{
    return undefined;
}

// ---------------- ChunkOpen ----------------
function ChunkOpen()
{
    var self = this;
}
ChunkOpen.prototype = new Chunk;
ChunkOpen.prototype.getType = function()
{
    return 'ChunkOpen';
}
ChunkOpen.prototype.i_createUi = function()
{
    g_deep++;
    return undefined;
}

// ---------------- ChunkClose ----------------
function ChunkClose()
{
    var self = this;
}
ChunkClose.prototype = new Chunk;
ChunkClose.prototype.getType = function()
{
    return 'ChunkClose';
}
ChunkClose.prototype.i_createUi = function()
{
    g_deep--;
    return undefined;
}

// ---------------- ChunkCheckProp ----------------
function ChunkCheckProp()
{
    var self = this;
}
ChunkCheckProp.prototype = new Chunk;
ChunkCheckProp.prototype.getType = function()
{
    return 'ChunkCheckProp';
}
ChunkCheckProp.prototype.i_createUi = function()
{
    var self        = this;
    var div         = $('<div/>');
    var prop_name   = $('<input/>');
    var prop_val    = $('<input/>');
    var condition   = $('<select/>');
    var vals_info   = [
        { 'title': '=='     },
        { 'title': '!='     },
        { 'title': '>'      },
        { 'title': '<'      },
    ];

    condition.change(function(a_ev){
        var target = $(a_ev.target);
    });

    for (var i = 0; i < vals_info.length; i++){
        var val_info = vals_info[i];
        var option   = $('<option>'+val_info.title+'</option>');
        condition.append(option);
    }

    div.append(prop_name);
    div.append(condition);
    div.append(prop_val);

    return div;
}

// ---------------- ChunkInt8 ----------------
function ChunkInt8()
{
    var self = this;
}
ChunkInt8.prototype = new Chunk;
ChunkInt8.prototype.getType = function()
{
    return 'ChunkInt8';
}
ChunkInt8.prototype.i_createUi = function()
{
    var self = this;
    var td   = $('<div>int8</div>');
    return td;
}

// ---------------- ChunkString ----------------
function ChunkString(){
    var self = this;
}
ChunkString.prototype = new Chunk;
ChunkString.prototype.getType = function()
{
    return 'ChunkString';
}
ChunkString.prototype.i_createUi = function()
{
    var self = this;
    var td   = $('<div>string</div>');
    return td;
}

// ---------------- ChunkCondition ----------------
function ChunkCondition()
{
    var self = this;
}
ChunkCondition.prototype = new Chunk;
ChunkCondition.prototype.getType = function()
{
    return 'ChunkCondition';
}
ChunkCondition.prototype.i_createUi = function()
{
    var self = this;
    var td   = $('<div>condition</div>');
    return td;
}

// ---------------- ChunkCondition ----------------
function ChunkCondition()
{
    var self = this;
}
ChunkCondition.prototype = new ChunkObjectSelect;
ChunkCondition.prototype.getType = function()
{
    return 'ChunkCondition';
}
ChunkCondition.prototype.i_createUi = function()
{
    var self = this;
    var td   = $('<div>condition</div>');
    return td;
}

