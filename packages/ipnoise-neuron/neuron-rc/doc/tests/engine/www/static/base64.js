// vim:syntax=javascript tabstop=4 expandtab

var Base64 = {
    _keyStr: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",

    encodeFromStringOfUint8: function(a_input){
        var self = this;

        var output = "";
        var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
        var i = 0;

        while (i < a_input.length){
            chr1 = a_input.charCodeAt(i++);
            chr2 = a_input.charCodeAt(i++);
            chr3 = a_input.charCodeAt(i++);

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
                +   self._keyStr.charAt(enc1)
                +   self._keyStr.charAt(enc2)
                +   self._keyStr.charAt(enc3)
                +   self._keyStr.charAt(enc4);
        }

        return output;
    },

    encodeFromUnicode: function(a_data){
        var data = string_of_unicode_to_string_of_uint8_utf8(
            a_data
        );
        return encodeFromStringOfUint8(data);
    },

    encode: function(a_data){
        return encodeFromUnicode(a_data);
    },

    // FYI: 1 unicode symbol = 2 bytes
    // decode every 1 byte to 1 symbol (2 bytes length)
    // example: decoded byte 0xff will returned 0x00ff symbol
    decodeToStringOfUint8: function(a_input){
        var self = this;

        var output = "";
        var chr1, chr2, chr3;
        var enc1, enc2, enc3, enc4;
        var i = 0;

        a_input = a_input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

        while (i < a_input.length){
            enc1 = self._keyStr.indexOf(a_input.charAt(i++));
            enc2 = self._keyStr.indexOf(a_input.charAt(i++));
            enc3 = self._keyStr.indexOf(a_input.charAt(i++));
            enc4 = self._keyStr.indexOf(a_input.charAt(i++));

            chr1 = (enc1        << 2) | (enc2 >> 4);
            chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
            chr3 = ((enc3 & 3)  << 6) | enc4;

            output += String.fromCharCode(chr1);

            if (enc3 != 64){
                output = output + String.fromCharCode(chr2);
            }
            if (enc4 != 64){
                output = output + String.fromCharCode(chr3);
            }
        }

        return output;
    },

    decodeToArrayBufferOfUint8: function(a_input){
        var res = Base64.decodeToStringOfUint8(a_input);
        var ab  = string_of_uint8_to_arrayBuffer_of_uint8(res);
        return ab;    
    },

    decodeToStringOfUnicode: function(a_input){
        var res     = Base64.decodeToStringOfUnicode(a_input);
        var unicode = stringOfUtf8ToStringOfUnicode(res);
        return unicode;
    },

    decode: function(a_input){
        return Base64.decodeToStringOfUnicode(a_input);
    }
}

