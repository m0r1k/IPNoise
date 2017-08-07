function addItemWindowClass(args)
{
    var context = args["context"]; 
    if (!context){
        perror("Missing argument 'args.context'");
        return null;
    }
 
    var context_doc = context.document;
    var self = AbstractClass("WindowClass", args);

    var group_id = args["group_id"];

    self.setTitle("Dialog with '"+name+"'");
 
    var tmpl = TemplateAddItemWindowClass();
    tmpl.onSubmit = function (args){
        self.close();
        var name  = args["name"];
        var descr = args["descr"];
        if (name.length && group_id.length){
            ipnoise_api.API_additem({
                "group_id":   group_id,
                "name":       name,
                "descr":      descr
            });
        }
    };

    self.setBodyElement(tmpl.main_div);
 
    return self;
}

