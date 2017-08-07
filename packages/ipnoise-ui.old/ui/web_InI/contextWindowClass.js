/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Context windows class implementation
 *  Jun, 2009
 *
 */

var context_window = null;

function closeContextWindow()
{
    if (context_window){
        context_window.close();
    }
    context_window = null;    
}

function contextWindowClass(args)
{
    if (!args.context){
        perror("Missing argument 'context'");
        return null;
    }

    var elements = args.elements;

    args["hide-head"]       = 1;
    args["disable-move"]    = 1;
    args["disable-resize"]  = 1; 

    var self = AbstractClass("WindowClass", args);

    // store context window
    context_window = self;

    var context     = args.context;
    var context_doc = context.document;
    var table       = context_doc.createElement("table");
    var tbody       = context_doc.createElement("tbody");
    table.appendChild(tbody)

    self.setSize("auto", "auto");
    self.focus();

    var close_div = context_doc.createElement("div");
    close_div.setAttribute("id", "closeDiv");
    close_div.setAttribute("style",
         "width:     100%;"
        +"height:    100%;"
        +"position:  absolute;"
        +"z-index:   "+(top_zindex - 1)+";"
    );
    close_div.setAttribute("onclick", "closeContextWindow();");
    context_doc.getElementsByTagName("body")[0].appendChild(close_div);

    self.close = function (){
        cleanNode(table);
        cleanNode(close_div);
        self.WindowClass.close();
    };

    for (var i = 0; i < elements.length; i++){
        var img_path    = elements[i].img;
        var text        = elements[i].text;
        var class_name  = elements[i].class;
        var onclick     = elements[i].onclick;

        // create line
        var tr = context_doc.createElement("tr");
        tr.setAttribute("onclick", "closeContextWindow();"+onclick);
        tr.setAttribute("onmouseover", "this.setAttribute(\"class\", \"contextMenuItemSelected\");");
        if (class_name){
            tr.setAttribute("onmouseout",  "this.setAttribute(\"class\", \""+class_name+"\");");
        } else {
            tr.setAttribute("onmouseout",  "this.removeAttribute(\"class\");");
        }

        // set icon
        var td  = context_doc.createElement("td");
        if (img_path){
            var img = context_doc.createElement("img");
            img.setAttribute("src", img_path);
            td.appendChild(img);
        }
        tr.appendChild(td);

        // set text
        td = context_doc.createElement("td");
        td.appendChild(context_doc.createTextNode(text));
        tr.appendChild(td);

        tbody.appendChild(tr);
    };
    self.setInnerHtml("<table class=\"contextMenu\">"
        +table.innerHTML
        +"</table>"
    );

    return self;
};

