// vim:syntax=javascript tabstop=4 expandtab

function copy_args(a_dst, a_src)
{
    for (var key in a_src){
        var val = a_src[key];
        if ('params' == key){
            for (var cur_param_name in val){
                var cur_param_val = val[cur_param_name];
                a_dst.params[cur_param_name] = cur_param_val;
            }
        } else {
            a_dst[key] = val;
        }
    }
}

function degToRad(a_degrees)
{
    var ret = a_degrees * Math.PI / 180;
    return ret;
}

function magnitude(a_vec)
{
    var ret;

    ret = Math.sqrt(
            (a_vec[X] * a_vec[X])
        +   (a_vec[Y] * a_vec[Y])
        +   (a_vec[Z] * a_vec[Z])
    );

    return ret;
}

function normalize(a_vector)
{
    var vector = [
        a_vector[X],
        a_vector[Y],
        a_vector[Z]
    ];

    if (    1 < vector[X]
        ||  1 < vector[Y]
        ||  1 < vector[Z])
    {
        var magnitude = magnitude(vector);
        if (!magnitude){
            getLogger().pfatal(
                "magnitude 0 will cause"
                +" devision by zero\n",
                {
                    'vector':       vector,
                    'magnitude':    magnitude
                }
            );
        }
        vector[X] = vector[X] / magnitude;
        vector[Y] = vector[Y] / magnitude;
        vector[Z] = vector[Z] / magnitude;
    }

    return vector;
}

function JSON_STR_TO_VEC3(a_val)
{
    var ret = undefined;

    if ('string' == jQuery.type(a_val)){
        var array = JSON.parse(a_val);
        if ('array' == jQuery.type(array)){
            ret = vec3.create([
                array[0],
                array[1],
                array[2]
            ]);
        }
    }

    return ret;
}

// arrayBuffer of uint16 -> string of uint16
// input:   arrayBuffer of uint16, 1 item = 2 bytes
// output:  string of uint16, 1 symbol = 2 bytes = 1 item
function arrayBuffer_of_uint16_to_string_of_uint16(
    a_ab)
{
    var ret = String.fromCharCode.apply(
        null,
        new Uint16Array(a_ab)
    );

    return ret;
}

// blob -> array buffer
// input:   blob
// output:  arrayBuffer
function blob_to_arrayBuffer(
    a_blob,
    a_cb)
{
    var ret     = undefined;
    var reader  = new FileReader();

    reader.onload = function(a_ev){
        var target = a_ev.target;
        a_cb(target.result);
    };
    reader.readAsArrayBuffer(a_blob);
}

// arrayBuffer of uint8 -> array of uint8
// input:   arrayBuffer with uint8 items
// output:  array with uint8 items
function arrayBuffer_of_uint8_to_array_of_uint8(
    a_ab)
{
    var bufView = new Uint8Array(a_ab);
    var ret     = [];

    for (i = 0; i < bufView.length; i++){
        ret.push(bufView[i]);
    }

    return ret;
}

// blob of uint8 -> array of uint8
// input:   blob with uint8 items
// output:  array of uint8 items
function blob_of_uint8_to_array_of_uint8(
    a_blob,
    a_cb)
{
    var ab = blob_to_arrayBuffer(
        a_blob,
        function(a_result){
            var res = arrayBuffer_of_uint8_to_array_of_uint8(
                a_result
            );
            a_cb(res);
        }
    );
}

// blob of uint8 -> string of uint8
// input:   blob with uint8 items
// output:  string with uint8 items packed in 2 bytes
// example: input 0xff output will be 0x00ff,
// (because unicode symbol length = 2 bytes)
function blob_of_uint8_to_string_of_uint8(
    a_blob,
    a_cb)
{
    blob_of_uint8_to_array_of_uint8(
        a_blob,
        function(a_result){
            var res = array_of_uint8_to_string_of_uint8(
                a_result
            );
            a_cb(res);
        }
    );
}

// array of uint8 -> string of uint8
// input:   array of uint8 items
// output:  string with uint8 items packed in 2 bytes
// example: input 0xff output 0x00ff
function array_of_uint8_to_string_of_uint8(
    a_arr)
{
    var ret = '';
    var i;

    for (i = 0; i < a_arr.length; i++){
        ret += String.fromCharCode(a_arr[i]);
    }

    return ret;
}

// arrayBuffer of uint8 (utf8) -> string of unicode
// input:   arrayBuffer of uint8 items what present utf8 string
// output:  unicode string with uint16 items
//  (2 bytes per symbol in unicode)
// example:
//  input  2 bytes: 0xD0, 0xAF (utf8 code of "Я" is 0xD0AF)
//  output 2 bytes: 0x042F Unicode Cyrillic Capital Letter "Я"
function arrayBuffer_of_uint8_utf8_to_string_of_unicode(
    a_ab)
{
    var data = arrayBuffer_of_uint8_to_string_of_uint8(a_ab);
    var ret  = string_of_utf8_to_string_of_unicode(data);

    return ret;
}

// arrayBuffer of uint8 -> string of uint8
// input:   arrayBuffer of uint8 items
// output:  string with uint8 items packed in 2 bytes
// example: input 0xff output 0x00ff,
// (because 1 unicode symbol have 2 bytes length)
function arrayBuffer_of_uint8_to_string_of_uint8(
    a_ab)
{
    var arr  = arrayBuffer_of_uint8_to_array_of_uint8(a_ab);
    var ret  = array_of_uint8_to_string_of_uint8(arr);

    return ret;
}

// string of uint16 -> arrayBuffer of uint16
// input:   string of unicode 1 symbol = 2 bytes
// output:  arrayBuffer with uint16 items,
//   1 item = 1 symbol = 2 bytes
function string_of_uint16_to_arrayBuffer_of_uint16(
    a_str)
{
    // 2 bytes for each char
    var buf     = new ArrayBuffer(a_str.length * 2);
    var bufView = new Uint16Array(buf);
    var i;

    for (i = 0; i < a_str.length; i++){
        bufView[i] = a_str.charCodeAt(i);
    }

    return buf;
}

// string of unicode -> array of uint8 bytes (utf8)
// input:  unicode string, 2 bytes per symbol
// output: array of uint8 bytes,
//   what contain unicode to utf8 converted string,
//   1 output item = 1 byte
function string_to_array_of_utf8_uint8(
    a_str)
{
    var utf8 = [];
    var i;

    for (i = 0; i < a_str.length; i++){
        var charcode = a_str.charCodeAt(i);

        if (charcode < 0x80){
            utf8.push(charcode);
        } else if (charcode < 0x800){
            utf8.push(
                0xc0 | (charcode >> 6),
                0x80 | (charcode & 0x3f)
            );
        } else if ( charcode < 0xd800
            ||      charcode >= 0xe000)
        {
            utf8.push(
                0xe0 | (charcode >> 12),
                0x80 | ((charcode>>6) & 0x3f),
                0x80 | (charcode & 0x3f)
            );
        } else {
            // surrogate pair
            i++;
            // UTF-16 encodes 0x10000-0x10FFFF by
            // subtracting 0x10000 and splitting the
            // 20 bits of 0x0-0xFFFFF into two halves
            charcode = 0x10000 + (((charcode & 0x3ff) << 10)
                | (a_str.charCodeAt(i) & 0x3ff));

            utf8.push(
                0xf0 | (charcode  >> 18),
                0x80 | ((charcode >> 12)    & 0x3f),
                0x80 | ((charcode >> 6)     & 0x3f),
                0x80 | (charcode            & 0x3f)
            );
        }
    }

    return utf8;
}

// string of unicode to arrayBuffer of uint8 with utf8 bytes
// input:  unicode string
// output: arrayBuffer with uint8 items,
//   what contain utf8 encoded string
function string_to_arrayBuffer_of_utf8_uint8(
    a_data)
{
    var arr = string_to_string_of_uint8_utf8(a_data);
    var ret = string_of_uint8_to_arrayBuffer_of_uint8(arr);

    return ret;
}

// string of uint8 to arrayBuffer of uint8
// input:   string of uint8 items (2 bytes per item)
//   (because 1 unicode symbol have 2 bytes length)
// output:  arrayBuffer of uint8 items
// example: input 0xff12 output 0x12
//   (input data will be truncated from 2 to 1 byte)
function string_of_uint8_to_arrayBuffer_of_uint8(
    a_uint8)
{
    var buf     = new ArrayBuffer(a_uint8.length);
    var bufView = new Uint8Array(buf);
    var i;

    for (i = 0; i < a_uint8.length; i++){
        bufView[i] = a_uint8.charCodeAt(i);
    }

    return buf;
}

// array of uint8 (parts of utf8 string) -> string of unicode
// input:  array of uint8 bytes what present utf8 string
// output: utf8 -> unicode converted string
function array_of_uint8_utf8_to_string_of_unicode(
    a_array)
{
    var out, i, len, c;
    var char2, char3;

    out = "";
    len = a_array.length;
    i   = 0;

    while (i < len){
        c = a_array[i++];
        switch (c >> 4){
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                // 0xxxxxxx
                out += String.fromCharCode(c);
                break;

            case 12:
            case 13:
                // 110x xxxx   10xx xxxx
                char2 = a_array[i++];
                out   += String.fromCharCode(
                    ((c & 0x1F) << 6) | (char2 & 0x3F)
                );
                break;

            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                char2 = a_array[i++];
                char3 = a_array[i++];
                out   += String.fromCharCode(
                        ((c     & 0x0F) << 12)
                    |   ((char2 & 0x3F) << 6)
                    |   ((char3 & 0x3F) << 0)
                );
                break;
        }
    }

    return out;
}

// string of unicode -> string of uint8 items,
//   what present utf8 string
// input:  unicode string with uint16 items
//   (2 bytes per symbol)
// output: unicode string with uint8 items packed in 2 bytes
//   (encoded in utf8)
// example:
//  input  2 bytes: 0x042F Unicode Cyrillic Capital Letter "Я"
//  output 4 bytes: 0x00D0, 0x00AF (utf8 code of "Я" is 0xD0AF)
function string_to_string_of_uint8_utf8(
    a_string)
{
    var string  = a_string.replace(/\r\n/g,"\n");
    var utf8    = "";

    for (var n = 0; n < string.length; n++){
        var c = string.charCodeAt(n);
        if (c < 128) {
            utf8 += String.fromCharCode(c);
        } else if((c > 127) && (c < 2048)){
            utf8 += String.fromCharCode((c >> 6) | 192);
            utf8 += String.fromCharCode((c & 63) | 128);
        } else {
            utf8 += String.fromCharCode((c >> 12) | 224);
            utf8 += String.fromCharCode(((c >> 6) & 63) | 128);
            utf8 += String.fromCharCode((c & 63) | 128);
        }
    }

    return utf8;
}

// string of uint8 items packed in 2 bytes (utf8) -> string of unicode
// input:  unicode string with uint8 items packed in 2 bytes
//  (encoded in utf8)
// output: unicode string with uint16 items
//  (2 bytes per symbol in unicode)
// example:
//  input  4 bytes: 0x00D0, 0x00AF (utf8 code of "Я" is 0xD0AF)
//  output 2 bytes: 0x042F Unicode Cyrillic Capital Letter "Я"
function string_of_utf8_to_string_of_unicode(
    a_utf8)
{
   var string = "";
   var i = 0;
   var c = c1 = c2 = 0;

    while (i < a_utf8.length){
        c = a_utf8.charCodeAt(i);
        if (c < 128){
            string += String.fromCharCode(c);
            i++;
        } else if ( c > 191
            &&      c < 224)
        {
            c2 = a_utf8.charCodeAt(i+1);
            string += String.fromCharCode(
                    ((c & 31) << 6)
                |   (c2 & 63)
            );
            i += 2;
        } else {
            c2 = a_utf8.charCodeAt(i+1);
            c3 = a_utf8.charCodeAt(i+2);
            string += String.fromCharCode(
                    ((c & 15)  << 12)
                |   ((c2 & 63) << 6)
                |   (c3 & 63)
            );
            i += 3;
        }
    }

    return string;
}

// url
function getUrlParameter(
    a_name)
{
    var res;
    var ret = undefined;

    res = RegExp(a_name + '=' + '(.+?)(&|$)').exec(
        location.search
    );
    if (    res
        &&  1 <= res.length)
    {
        ret = decodeURI(res[1]);
    }

    return ret;
}

// object
function getCurObjectId()
{
    var object_id = getUrlParameter('object_id');
    return object_id;
}

function addrToFloat(a)
{
    var ret = parseFloat('0'
        + a.replace(/^\D+/, "1000")
            .replace(/\D/g, ",")
            .replace(/,/,   ".")
            .replace(/,/g,  "")
    );
    return ret;
}

function sortAddr(a, b)
{
    var ret = 0;
    var aa = addrToFloat(a.name);
    var bb = addrToFloat(b.name);
    ret = aa - bb;
    if (ret < 0){
        ret = -1;
    } else if (ret > 0){
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

// getAttributes - return all element attributes
//
// >>
// << array with attributes
//
(function($){
    $.fn.getAttributes = function(){
        var self = this;
        var attributes = {};
        if (self.length){
            $.each(this[0].attributes,
                function(index, attr){
                    attributes[attr.name] = attr.value;
                }
            );
        }
        return attributes;
    };
})(jQuery);

String.prototype.capitalize = function(){
    var self = this;
    var ret  = self.charAt(0).toUpperCase() + self.slice(1);
    return ret;
}

