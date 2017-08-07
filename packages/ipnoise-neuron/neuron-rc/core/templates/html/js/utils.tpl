// vim:syntax=javascript tabstop=4 expandtab

/**
*
*  Base64 encode / decode
*  http://www.webtoolkit.info/
*
**/

var Base64 = {
    // private property
    _keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",

    // public method for encoding
    encode : function (input){
        var self = this;

        var output = "";
        var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
        var i = 0;

        input = Base64._utf8_encode(input);

        while (i < input.length){
            chr1 = input.charCodeAt(i++);
            chr2 = input.charCodeAt(i++);
            chr3 = input.charCodeAt(i++);

            enc1 = chr1 >> 2;
            enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
            enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
            enc4 = chr3 & 63;

            if (isNaN(chr2)){
                enc3 = enc4 = 64;
            } else if (isNaN(chr3)){
                enc4 = 64;
            }

            output = output
                +   self._keyStr.charAt(enc1) + self._keyStr.charAt(enc2)
                +   self._keyStr.charAt(enc3) + self._keyStr.charAt(enc4);
        }
        return output;
    },

    // public method for decoding
    decode : function (input){
        var self = this;

        var output = "";
        var chr1, chr2, chr3;
        var enc1, enc2, enc3, enc4;
        var i = 0;

        input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

        while (i < input.length) {
            enc1 = self._keyStr.indexOf(input.charAt(i++));
            enc2 = self._keyStr.indexOf(input.charAt(i++));
            enc3 = self._keyStr.indexOf(input.charAt(i++));
            enc4 = self._keyStr.indexOf(input.charAt(i++));

            chr1 = (enc1 << 2) | (enc2 >> 4);
            chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
            chr3 = ((enc3 & 3) << 6) | enc4;

            output = output + String.fromCharCode(chr1);

            if (enc3 != 64){
                output = output + String.fromCharCode(chr2);
            }
            if (enc4 != 64){
                output = output + String.fromCharCode(chr3);
            }
        }
        output = Base64._utf8_decode(output);
        return output;
    },

    // private method for UTF-8 encoding
    _utf8_encode : function (string) {
        string = string.replace(/\r\n/g,"\n");
        var utftext = "";

        for (var n = 0; n < string.length; n++){
            var c = string.charCodeAt(n);
            if (c < 128) {
                utftext += String.fromCharCode(c);
            } else if((c > 127) && (c < 2048)){
                utftext += String.fromCharCode((c >> 6) | 192);
                utftext += String.fromCharCode((c & 63) | 128);
            } else {
                utftext += String.fromCharCode((c >> 12) | 224);
                utftext += String.fromCharCode(((c >> 6) & 63) | 128);
                utftext += String.fromCharCode((c & 63) | 128);
            }
        }

        return utftext;
    },

    // private method for UTF-8 decoding
    _utf8_decode : function (utftext){
        var string = "";
        var i = 0;
        var c = c1 = c2 = 0;

        while (i < utftext.length){
            c = utftext.charCodeAt(i);
            if (c < 128) {
                string += String.fromCharCode(c);
                i++;
            } else if((c > 191) && (c < 224)) {
                c2 = utftext.charCodeAt(i+1);
                string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
                i += 2;
            } else {
                c2 = utftext.charCodeAt(i+1);
                c3 = utftext.charCodeAt(i+2);
                string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
                i += 3;
            }
        }
        return string;
    }
}

// url
function getUrlParameter(name){
    var ret = decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(
            location.search
        )||[,null])[1]
    );
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

function uiGetObject(a_el)
{
    var div         = $(a_el).closest('.object');
    var object_id   = div.attr('id');
    var object      = null;

    if (object_id){
        object = getObjectById(object_id);
    }

    return object;
}

function delObject(a_el)
{
    var object = uiGetObject(a_el);
    if (object){
        object.deleteWithConfirm();
    }
}

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

