function historyWindowClass(args)
{
    var context = args["context"]; 
    if (!context){
        perror("Missing argument 'args.context'");
        return null;
    }


    var context_doc = context.document;
    var self = AbstractClass("WindowClass", args);

    self.huid = args["huid"]; 
    if (!self.huid){
        perror("Missing argument 'args.huid'");
        cleanClass(self);
        return null;
    }
 
    self.getHistory = function(args){
        var data = {
            huid:   self.huid,
            chatid: args.chatid,
        }
        if (typeof args.msg != 'undefined'){
            data["msg"] = Base64.encode(args.msg);
        }
        ipnoise_api.API_searchhistory(data);
    };

    var tmpl = TemplateHistoryWindowClass({
        "getHistory":   self.getHistory,
    });

    self.setBodyElement(tmpl.main_div);

    self.setSize = function (x, y){
        tmpl.setSize(x - 10, y - 10);
        self.WindowClass.setSize(x, y);
    }

    self.setHistory = function (chats){
        return tmpl.setHistory(chats);
    }

    self.setSize("500", "300");

    return self;
}

