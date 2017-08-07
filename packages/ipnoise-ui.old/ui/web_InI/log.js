/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Log class implementation
 *  Aug, 2009.
 */

var select_class_id = null;

function insertAfter(parent, node, referenceNode)
{
    parent.insertBefore(node, referenceNode.nextSibling);
};

function dumpStack ()
{
    var callstack = [];
    try {
        i.dont.exist += 0; //does not exist - that's the point
    } catch(e) {
        if (e.stack) {
            var lines = e.stack.split("\n");
            for (var i = 0, len = lines.length; i < len; i++) {
                if ( lines[i].match( /^\s*[A-Za-z0-9\-_\$]+\(/ ) ) {
                    callstack.push(lines[i]);
                }
            }
            //Remove call to printStackTrace()
            callstack.shift();
        }
    }
    return callstack;
}

function text_align (value, length) {
    return do_align(value, length, "&nbsp;", 1);
};

function text_int_align (value, length) {
    return do_align(value, length, "0", 0);
};

function do_align (value, length, align_symbol, align_position) {
    var val = ""  + value;
    var cur_len   = val.length;
    var align_len = length - cur_len;
    var ret       = value;
    if (align_len > 0){
        for (var i = 0; i < align_len; i++){
            if (align_position == 0){
                ret = align_symbol + ret;
            } else {
                 ret = ret + align_symbol;
            }
        }
    }
    return ret;
};

function plog (type, msg, detail, class_id)
{
    var log_to = "";
    if (window.console){
        log_to == "firebug"
    }

    var cur_date = new Date();
    var year  = cur_date.getFullYear();
    var month = cur_date.getMonth();
    var day   = cur_date.getDate();
    var hour  = cur_date.getHours();
    var min   = cur_date.getMinutes();
    var sec   = cur_date.getSeconds();
    var msec   = cur_date.getMilliseconds();

    year  = text_int_align(year,  4);
    month = text_int_align(month, 2);
    day   = text_int_align(day,   2);
    hour  = text_int_align(hour,  2);
    min   = text_int_align(min,   2);
    sec   = text_int_align(sec,   2);
    msec  = text_int_align(msec,  3);

    var stack_array  = dumpStack() || [];
    var stack_string = "";
    for (var i in stack_array){
        stack_string = stack_string + "    "+stack_array[i]+"\n";
    }

    var label;
    var log_as_line = "";

    var log_date = year + "-" + month + "-" + day + " "
        + hour + ":" + min + ":" + sec + "." + msec

    // date
    log_as_line += log_date+" ";

    // type of message
    log_as_line += type+" ";

    var func = "?";
    if (stack_array.length > 2){
        func = stack_array[2].split("(")[0];
    }

    // function
    log_as_line += func+" ";

    // class ID
    if (class_id){
        log_as_line += class_id+" ";
    }

    // message
    log_as_line += msg+" ";

    // detail
    var detail_msg = "";
    if (typeof detail == "object"){
        for (var key in detail){
            detail_msg += key + ": " + detail[key] + "\n";
        }
    } else {
        detail_msg = detail;
    }
    log_as_line += "\nDetail:\n"+detail_msg;

    // stack
    log_as_line += "\nStack:\n"+stack_string;

    if (log_to == "firebug"){
        console.log(log_as_line);
    } else {
       //  alert (log_as_line);
    }
};

function pwarn (msg, detail, class_id) {
    plog("WARNING", msg, detail, class_id);
};
function pdebug (msg, detail, class_id) {
    plog("DEBUG", msg, detail, class_id);
};
function perror (msg, detail, class_id) {
    plog("ERROR", msg, detail, class_id);
};
function pinfo (msg, detail, class_id) {
    plog("INFO", msg, detail, class_id);
};

