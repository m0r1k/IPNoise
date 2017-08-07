/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Main window class implementation
 *  Aug, 2009.
 */

function groupClicked(element)
{
    var deep = -1;
    var img  = element;
    var node = element;    
    while (node){
        var tmp_deep = node.getAttribute("deep");
        if (typeof(tmp_deep) != "undefined"
            && tmp_deep != null)
        {
            deep = tmp_deep;
            break;
        }
        node = node.parentNode;
    }
    if (deep < 0){
        return;   
    }
    var is_expanded = node.getAttribute("is_expanded");
    if (typeof(is_expanded) == "undefined"
        || is_expanded == null)
    {
        is_expanded = 0;
    }

    if (is_expanded == 1){
        is_expanded = 0; 
    } else {
        is_expanded = 1;
    }

    node.setAttribute("is_expanded", is_expanded);

    if (is_expanded){
        img.setAttribute("src", "images/collapsebtn2.gif");
    } else {
        img.setAttribute("src", "images/expandbtn2.gif");
    }

    node = node.nextSibling;
    while (node){
        var tmp_deep = node.getAttribute("deep");
        if (typeof(tmp_deep) != "undefined"
            && tmp_deep != null)
        {
            if (tmp_deep <= deep){
                break;
            }
            if (is_expanded){
                node.style.display = "";
            } else {
                node.style.display = "none";
            }
        }
        node = node.nextSibling;
    }

}

function createTreeElement(args)
{
    var ret = null;

    if (typeof(args.is_expanded) == "undefined"
            || args.is_expanded == null)
    {
        args.is_expanded = 0;
    }

    if (args.type == "group"){
        ret = createGroupElement(args);
    } else {
        ret = createItemElement(args);
    }
    return ret;
};

function createItemElement(args)
{
    var table;
    var tr;    
    var td;
    var img;
    var ret;

    var context_doc = args.context.document;

    // line1
    tr = context_doc.createElement("div");
    tr.setAttribute("class", "row");
    tr.setAttribute("deep", args.deep);
    tr.setAttribute("type", args.type);
    tr.setAttribute("name", args.name);
    tr.setAttribute("huid", args.huid);
    if (typeof args.group_id != 'undefined'){
        tr.setAttribute("group_id", args.group_id);
    }
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    tr.appendChild(td);
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
    );
    ret = tr;

    table = context_doc.createElement("div");
    table.setAttribute("class", "div");
    table.setAttribute("colspacing", "0");
    table.setAttribute("colpadding", "0");
    table.setAttribute("border",      "0");
    table.setAttribute("style",
         "background: rgb(214, 223, 247)"
            +" url(images/blank.gif) repeat scroll 0% 0%;"
        +"cursor: pointer;"
        +"width:  100%;"
    );
    td.appendChild(table);

    // subline
    tr = context_doc.createElement("div");
    tr.setAttribute("class", "row");
    table.appendChild(tr);

    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style", "margin: 0px; padding: 0px");
    tr.appendChild(td);

    // table
    table = context_doc.createElement("table");
    table.setAttribute("colspacing", "0");
    table.setAttribute("colpadding", "0");
    table.setAttribute("border",      "0");
    table.setAttribute("style",
         "background: rgb(214, 223, 247)"
            +" url(images/blank.gif) repeat scroll 0% 0%;"
        +"cursor: pointer;"
        +"width:  100%;"
    );
    td.appendChild(table);
 
    // subline (ITEM CONTENT)
    tr = context_doc.createElement("div");
    tr.setAttribute("class", "row");
    table.appendChild(tr);

    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "1");
    img.setAttribute("width",  "2");
    img.setAttribute("border", "0");
    td.appendChild(img);

    // td
    for (var i = 0; i < args.deep; i++){
        td = context_doc.createElement("div");
        td.setAttribute("class", "col");
        td.setAttribute("style",
             "margin:  0px;"
            +"padding: 0px;"
        );
        tr.appendChild(td);
        img = context_doc.createElement("img");
        img.setAttribute("src",    "images/blank.gif");
        img.setAttribute("height", "1");
        img.setAttribute("width",  "20");
        img.setAttribute("border", "0");
        td.appendChild(img);
    }
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:       0px;"
        +"padding:      0px;"
        +"background:   transparent url() repeat-y scroll 0% 0%"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "9");
    img.setAttribute("width",  "9");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
        +"background-repeat: repeat-x;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "1");
    img.setAttribute("width",  "5");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    args.status_icon);
    img.setAttribute("height", "15");
    img.setAttribute("width",  "21");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
        +"background-repeat: repeat-x;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "1");
    img.setAttribute("width",  "5");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
        +"width:   100%;"
    );
    td.setAttribute("height", "22");
    td.setAttribute("nowrap", "nowrap");
    td.setAttribute("align",  "left");
    tr.appendChild(td);
    span = context_doc.createElement("span");
    span.setAttribute("onclick",
        "createMsgWindow({ "
            +" \"huid\":         \""+args.huid+"\","
            +" \"name\":         \""+args.name+"\","
            +" \"sync_history\": 1,"
            +" \"show\":         1,"
        +"});"
    );
    span.setAttribute("style",
         'color:            rgb(33, 93, 198);'
        +'font-family:      Tahoma;'
        +'font-style:       normal;'
        +'font-variant:     normal;'
        +'font-weight:      normal;'
        +'font-size:        8pt;'
        +'line-height:      normal;'
        +'font-size-adjust: none;'
        +'font-stretch:     normal;'
        +'text-decoration:  none;'
    );
    td.appendChild(span);
    span.appendChild(context_doc.createTextNode(args.name));
    return ret;
}

function createGroupElement(args)
{
    var table;
    var tr;    
    var td;
    var img;
    var ret;

    var context_doc = args.context.document;

    // line1
    tr = context_doc.createElement("div");
    tr.setAttribute("class",        "row");
    tr.setAttribute("deep",         args.deep);
    tr.setAttribute("type",         args.type);
    tr.setAttribute("name",         args.name);
    tr.setAttribute("group_id",     args.group_id);
    tr.setAttribute("is_expanded",  args.is_expanded);
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    tr.appendChild(td);
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
    );
    ret = tr;

    table = context_doc.createElement("div");
    table.setAttribute("class", "grid");
    table.setAttribute("colspacing", "0");
    table.setAttribute("colpadding", "0");
    table.setAttribute("border",      "0");
    table.setAttribute("style",
         "background: rgb(214, 223, 247)"
            +" url(images/blank.gif) repeat scroll 0% 0%;"
        +"cursor: pointer;"
        +"width:  100%;"
    );
    td.appendChild(table);

    // subline
    tr = context_doc.createElement("div");
    tr.setAttribute("class", "row");
    table.appendChild(tr);

    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style", "margin: 0px; padding: 0px");
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "1");
    img.setAttribute("width",  "2");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    for (var i = 0; i < args.deep; i++){
        td = context_doc.createElement("div");
        td.setAttribute("class", "col");
        td.setAttribute("style",
             "margin:  0px;"
            +"padding: 0px;"
        );
        tr.appendChild(td);
        img = context_doc.createElement("img");
        img.setAttribute("src",    "images/blank.gif");
        img.setAttribute("height", "1");
        img.setAttribute("width",  "20");
        img.setAttribute("border", "0");
        td.appendChild(img);
    }
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:     0px;"
        +"padding:    0px;"
        +"background: transparent url() repeat-y scroll 0% 0%;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    if (args.is_expanded){
        img.setAttribute("src",    "images/collapsebtn2.gif");
    } else {
        img.setAttribute("src",    "images/expandbtn2.gif");
    }
    img.setAttribute("style",   "cursor: pointer;");
    img.setAttribute("height",  "9");
    img.setAttribute("width",   "9");
    img.setAttribute("border",  "0");
    img.setAttribute("onclick", "groupClicked(this);");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
        +"background-repeat: repeat-x;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "1");
    img.setAttribute("width",  "5");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    args.group_icon);
    img.setAttribute("height", "15");
    img.setAttribute("width",  "21");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
        +"background-repeat: repeat-x;"
    );
    tr.appendChild(td);
    img = context_doc.createElement("img");
    img.setAttribute("src",    "images/blank.gif");
    img.setAttribute("height", "1");
    img.setAttribute("width",  "5");
    img.setAttribute("border", "0");
    td.appendChild(img);
    // td
    td = context_doc.createElement("div");
    td.setAttribute("class", "col");
    td.setAttribute("style",
         "margin:  0px;"
        +"padding: 0px;"
        +"width:   100%;"
        +"background-repeat: repeat-x;"
    );
    td.setAttribute("height", "22");
    td.setAttribute("nowrap", "nowrap");
    td.setAttribute("align",  "left");
    tr.appendChild(td);
    span = context_doc.createElement("span");
    var style  = "color: rgb(33, 93, 198);";
        style += "font-family:      Tahoma;";
        style += "font-style:       normal;";
        style += "font-variant:     normal;"
        style += "font-weight:      normal;";
        style += "font-size:        8pt;";
        style += "line-height:      normal;";
        style += "font-size-adjust: none;";
        style += "font-stretch:     normal;";
        style += "text-decoration:  none;";

    span.setAttribute("style", style);
    td.appendChild(span);
    span.appendChild(context_doc.createTextNode(args.name));

    return ret;
}

function addItem(args)
{
    var group_id    = args["group_id"];
    var group_name  = args["group_name"]; 

    context_win.close();

    args["context"] = window;

    var win = addItemWindowClass(args);
    win.setTitle("Добавление заметки в группу '"+group_name+"'");
    win.focus();
}

function itemClass()
{
    var self = AbstractClass();

    var huid        = null;
    var name        = null;
    var descr       = null;
    var status_icon = null;
    var avatar_icon = null;
   
    self.setHuid = function(new_value){
        huid = new_value;
    }
    self.getHuid = function(){
        return huid;
    }
    //
    self.setName = function(new_value){
        name = new_value;
    }
    self.getName = function(){
        return name;
    }
    //
    self.setDescr = function(new_value){
        descr = new_value;
    }
    self.getDescr = function(){
        return descr;
    }
    //
    self.setStatusIcon = function(new_value){
        status_icon = new_value;
    }
    self.getStatusIcon = function(){
        return status_icon;
    }
    //
    self.setAvatarIcon = function(new_value){
        avatar_icon = new_value;
    }
    self.getAvatarIcon = function(){
        return avatar_icon;
    }
    //
    return self;
}

function mainWindowClass(args)
{
    if (!args.context){
        perror("Missing argument 'context'");
        return null;
    }

    var self = AbstractClass("WindowClass", args);

    var context      = args.context;
    var context_doc  = context.document;
    var main_div     = context_doc.createElement("div");
    main_div.setAttribute("class", "grid");
    main_div.setAttribute("style", "width: 100%;");

    var tmpl         = TemplateMainWindowClass();
    tmpl.onWhoAround = function (){
        // request who-around function
        var win = whoAroundWindowClass(args);
        win.setSize("500", "300"); 
        win.setTitle("Поиск пользователей");
        win.setAbsPos("300", "300");
        win.focus();
    }; 
    function fillGroups(group, items, deep, group_id){
        if (!deep){
            deep = 0;
        }
        var tr;
        while (group){
            if (/^group$/i.test(group.tagName)){
                tr = createTreeElement({
                    context:        context,
                    type:           "group",
                    name:           group.getAttribute("name"),
                    deep:           deep,
                    group_icon:     group.getAttribute("icon"),
                    group_id:       group.getAttribute("id"),
                    is_expanded:    1
                });
                main_div.appendChild(tr);
                var sub_group = group.firstChild;
                if (sub_group){
                    fillGroups(sub_group, items, deep + 1, group.getAttribute("id"));
                }
            } else if (/^item$/i.test(group.tagName)){
                var item_id         = group.getAttribute("huid");
                var item            = items[item_id];
                var element_info    = {
                    context:        context,
                    type:           "item",
                    name:           item.getAttribute("name"),
                    huid:           item.getAttribute("huid"),
                    deep:           deep + 1,
                    status_icon:    item.getAttribute("status_icon")
                }
                if (typeof group_id != 'undefined'){
                    element_info["group_id"] = group_id;
                }
                tr = createTreeElement(element_info);

                main_div.appendChild(tr);              
            }
            group = group.nextSibling;
        }
    }
    self.showContextMenu = function (ev){
        var node        = ev.target;
        var type        = "";
        var group_id    = ""; 
        var huid        = "";
        var name        = ""; 
        while (node){
            if (/\s*row\s*/.test(node.className)){
                type = node.getAttribute("type");
                if (type == "group" || type == "item"){
                    name     = node.getAttribute("name");
                    huid     = node.getAttribute("huid");
                    group_id = node.getAttribute("group_id");
                    break;
                }
                type = "";
            }
            node = node.parentNode;
        }

        if (type == ""){
            var elements = [
                {
                    "img":      "",
                    "text":     "Добавить",
                    "class":    "contextMenuAdd",
                    "onclick":  "alert('add');"
                },
            ];
             // context_win must be global
            context_win = contextWindowClass({
                "context":          context,
                "ev":               ev,
                "elements":         elements
            });
        }

        if (type == "group"){
            var elements = [
                {
                    "img":      "images/icon-plus.png",
                    "text":     "Добавить группу",
                    "class":    "contextMenuAddGroup",
                    "onclick":  "alert('Добавить группу');"
                },
                {
                    "img":      "images/icon-plus.png",
                    "text":     "Добавить заметку",
                    "class":    "contextMenuAddGroup",
                    "onclick":  "addItem({"
                               +"   group_id:   \""+group_id+"\","
                               +"   group_name: \""+name+"\","
                               +"});"
                }, 
                {
                    "img":      "images/icon-minus.png",
                    "text":     "Удалить группу",
                    "class":    "contextMenuDelGroup",
                    "onclick":  "alert('Удалить группу');"
                },
            ];
            // context_win must be global
            context_win = contextWindowClass({
                "context":          context,
                "ev":               ev,
                "elements":         elements
            });

        } else if (type == "item"){
            var elements = [
                {
                    "img":      "",
                    "text":     "Переименовать",
                    "class":    "contextMenuAddItem",
                    "onclick":  "alert('rename');"
                },
                {
                    "img":      "",
                    "text":     "Удалить из текущей группы",
                    "class":    "contextMenuDelItem",
                    "onclick":  "ipnoise_api.API_delitem({"
                                +" huid:        '"+huid+"',"
                                +" group_id:    '"+group_id+"'"
                                +"});"
                },
                {
                    "img":      "",
                    "text":     "Удалить из всех групп",
                    "class":    "contextMenuDelItem",
                    "onclick":  "ipnoise_api.API_delitem({"
                                +" huid:        '"+huid+"',"
                                +"});"
                },
            ];
            // context_win must be global
            context_win = contextWindowClass({
                "context":          context,
                "ev":               ev,
                "elements":         elements                
            });
        }
        return false;
    }
    self.setContactList = function (element){
        // remove childs
        while (main_div.firstChild)
            main_div.removeChild(main_div.firstChild);

        var groups  = element.getElementsByTagName("groups")[0];
        var items1  = element.getElementsByTagName("items")[0];
        if (items1){
            items1 = items1.getElementsByTagName("item");
        }
        var items = {};
        for (var i = 0; i < items1.length; i++){
            var item        = items1[i];
            var item_id     = item.getAttribute("huid");
            items[item_id]  = item;
        }

        var tr;
        var group = groups.getElementsByTagName("group")[0];
        
        if (!groups || !group){
            // show without groups
            for (var i = 0; i < items1.length; i++){
                tr = createTreeElement({
                    context:        context,
                    type:           "item",
                    name:           items1[i].getAttribute("name"),
                    huid:           items1[i].getAttribute("huid"),
                    deep:           0,
                    status_icon:    items1[i].getAttribute("status_icon")
                });
                main_div.appendChild(tr);
            }
            self.setInnerHtml("<div class='grid' width='100%'>"+div.innerHTML+"</div>");
            return;
        }

        fillGroups(groups.firstChild, items);

//        tmpl.main_div.append("<div class='grid' width='100%'>"+main_div.innerHTML+"</div>");
        tmpl.main_div.find(".contactList").append($(main_div));
        self.setSize("auto", "auto");
    }

    self.startBlink = function (huid){
        var rows = $("#"+self.getClassGuiId()).find(".row");
        for (var i = 0; i < rows.length; i++){
            var item_huid = rows[i].getAttribute("huid");
            if (item_huid && item_huid == huid){
                rows[i].setAttribute(
                    "style", "text-decoration: blink"
                );
                break;
            }
        }
    };

    self.stopBlink = function (huid){
        var rows = $("#"+self.getClassGuiId()).find(".row");
        for (var i = 0; i < rows.length; i++){
            var item_huid = rows[i].getAttribute("huid");
            if (item_huid && item_huid == huid){
                rows[i].setAttribute(
                    "style", ""
                );
                break;
            }
        }
    };

    self.setBodyElement(tmpl.main_div);

    return self;
}

