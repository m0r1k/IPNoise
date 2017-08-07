function whoAroundWindowClass(args)
{
    var context = args["context"]; 
    if (!context){
        perror("Missing argument 'args.context'");
        return null;
    }

    var context_doc = context.document;
    var self = AbstractClass("WindowClass", args);

    self.doSearch = function(){
        ipnoise_api.API_whoaround(); 
    }

    var tmpl = TemplateWhoAroundWindowClass({});
    tmpl.doSearch = self.doSearch;

    self.setSize = function (x, y){
        tmpl.setSize(x - 10, y - 10);
        self.WindowClass.setSize(x, y);
    }

    self.setResults = function(items){
        tmpl.setResults(items);
    }
    self.showContextMenu = function (ev){
        var huid = null;
        var node = ev.target;
        while (node){
            if (node.className && /item/.test(node.className)){
                huid = node.getAttribute("huid");
                break;
            }
            node = node.parentNode;
        }
        if (!huid){
            return true;
        }
        var elements = [
            {
                "img":      "",
                "text":     "Добавить в контакт лист",
                "class":    "contextMenuAddItem",
                "onclick":  "ipnoise_api.API_additem({"
                        +" huid:        '"+huid+"'"
                        +"});"
            },
        ];
        // context_win must be global
        context_win = contextWindowClass({
            "context":          context,
            "ev":               context.window.event || ev,
            "elements":         elements
        });           
        return false;
    }
    self.setBodyElement(tmpl.main_div);
    self.setSize("500", "300");
    self.doSearch();
    return self;
}

