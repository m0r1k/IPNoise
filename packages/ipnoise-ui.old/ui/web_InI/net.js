/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Network class implementation
 *  Aug, 2009.
 */

function XmlHttpRequestClass ()
{
    var self = AbstractClass();

    // Public
    self.url                = null; // document load url
    self.onload             = null; // user handler on load
    self.useJSON            = 0;    // convert data to JSON before send
                                    // and from JSON before call callback 
    self.do_not_show_errors = 0;    // don't show errors

    self.non_block          = 0;    // read not full data

    // self.publicData - used for store user public data
    // and defined in AbstractClass()

    // Private
    var xmlhttp     = null; // XMLHttpRequest object
    var data        = null; // answer from server

    var position    = 0;

    self.sendPost = function (data){
        self.send("POST", data);
    };
    self.sendGet = function (data){
        self.send("GET", data);
    };
    self.send = function (method, data){
        if (method != "GET" && method != "POST"){
            self.error("Unsupported method: '"+method+"'");
            return null;
        }
        if (self.useJSON){
            data = JSON.stringify(data);
        }
        self.xmlhttp = new XMLHttpRequest();
        if (self.non_block){
            self.xmlhttp.timeout = 3600*1000; // ms
        }
        self.xmlhttp.onreadystatechange = self.stateChanged;
        self.xmlhttp.open(method, self["url"], true);
        self.xmlhttp.send(data);
    };
    self.stateChanged = function(){
        if (self.non_block && self.onload && self.xmlhttp){
            if (self.xmlhttp.readyState == 4){
                // disconnect detected
                // if status_code 500 then connection was refused by ipnoise server,
                // because authentication required
                 data = {
                    status:         'DISCONNECT',
                    status_code:    self.xmlhttp.status
                };
                position = 0;
                self.onload(data);
                data = null;               
            } else if (self.xmlhttp.readyState == 3 && self.xmlhttp.responseText){
                // if we have onLoad handler call it with data
                data = {
                    status:       'SUCCESS',
                    responseXML:  self.xmlhttp.responseXML,
                    responseText: self.xmlhttp.responseText.substring(position, self.xmlhttp.responseText.length),
                    JSON:         null,
                    publicData:   self.publicData
                };
                position = self.xmlhttp.responseText.length;
                self.onload(data);
                data = null;
            };
        } else if (self.xmlhttp.readyState == 4){

            // important, clear variables before it will used :)
            
            // 4 - "loaded"
            if (self.xmlhttp.status == 200){
                // HTTP status: 200 = OK
                // ***** SUCCESS *****
                if (self.onload){
                    // if we have onLoad handler call it with data
                    data = {
                        status:       'SUCCESS',
                        responseXML:  self.xmlhttp.responseXML,
                        responseText: self.xmlhttp.responseText,
                        JSON:         null,
                        publicData:   self.publicData
                    };
                    if (self.useJSON){
                        data.JSON = JSON.parse(data.responseText);
                    }
                    self.onload(data);
                };
                data = null;
            } else {
                // ***** FAILED *****
                if (self.onload){
                    // if we have onLoad handler call it
                    data = {
                        status:       'FAILED',
                        responseXML:  null,
                        responseText: null,
                        JSON:         null,
                        publicData:   self.publicData
                    };
                    self.onload(data);
                };
                if (!self.do_not_show_errors){
                    self.error("Cannot read XML data from '"+self["url"]+"'",
                        "xmlhttp.status: "+self.xmlhttp.status
                    );
                }
                data = null;
            }
        }
    };

    // DO NOT FORGET RETURN SELF
    return self;
};

