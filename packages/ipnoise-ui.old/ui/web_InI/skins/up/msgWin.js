function TemplateMessageWindowClass()
{
    var self = AbstractClass();

    var default_width   = "100%";
    var default_height  = "100%";
   
    var cur_width       = default_width;
    var cur_height      = default_height;
  
    var cur_on_key_down     = "";
    var cur_on_show_history = "";

    var main_div_text = "";
     main_div_text += "<div class='msgWin grid'>";
     main_div_text += "    <div class='row'>";
     main_div_text += "       <div class='col'>";
     main_div_text += "            <div class='grid chatTabsPanel'>";
     main_div_text += "               <div class='row' style='height: 20px;'>";
     main_div_text += "                   <!-- chat tabs here -->";
     main_div_text += "               </div>";
     main_div_text += "            </div>";
     main_div_text += "       </div>";
     main_div_text += "      <div class='col controls_table'></div>";
     main_div_text += "    </div>";
     main_div_text += "    <!-- chat tabs body here -->";
     main_div_text += "</div>";

    self.main_div = $(main_div_text);

    self.setSize = function (width, height){
        if (!width || !height){
            perror("Invalid setSize request:"
                +" width: '"+width+"' height: '"+height+"'"
            );
            return 1;
        }
        cur_width  = width;
        cur_height = height;

        self.main_div.css("width",  cur_width);
        self.main_div.css("height", cur_height);
 
        self.main_div.find(".history_table_scroll").css("overflow", "auto"); 
        self.main_div.find(".history_table_scroll").css("width",    cur_width);
        self.main_div.find(".history_table_scroll").css("height",   cur_height - 110); 

        var input_height = "100px";
        self.main_div.find("#input_area").css("height", input_height);
        self.main_div.find("#input_area").css("width",  cur_width); 
    };
    self.selectChatById = function (src_chatid){
        self.main_div.find(".chattab").css("background", "#abb0c0");
        self.main_div.find(".chattabid_"+src_chatid).css("background", "#dee0d0");

        self.main_div.find(".chatbody").hide();//css("display",   "none");
        self.main_div.find(".chatbody_srcid_"+src_chatid).show();//css("display", "row");
        self.refresh();
    } 
    self.selectChat = function (element){
        var src_chatid = -1;
        var node = element;
        while (node){
            if (node.tagName == "DIV"){
                if (node.className){
                    var regexp = /chattabid_(\d+)/;
                    var res = regexp.exec(node.className);
                    if (res && res[1]){
                        src_chatid = res[1];
                        break;
                    }
                }
            }
            node = node.parentNode;
        }
        if (src_chatid >= 0){
            self.selectChatById(src_chatid);
        }
    }
    self.get_chattab_element = function (src_chatid, dst_chatid)
    {
        var chattab  = "<div class='col chattab chattabid_"+src_chatid+"'";
            chattab +=     "onclick=\"";
            chattab +=         "var class_obj = getClassByGuiId('"+self.getClassGuiId()+"');";
            chattab +=         "class_obj.selectChat(this);";
            chattab +=     "\">";
            chattab +=     "Чат #"+self.main_div.find(".chattab").length;
            chattab += "</div>";
        return $(chattab);
    }
    self.get_chattab_cont_element = function (src_chatid, dst_chatid)
    {
        var chattabcont  = "<div class='row chatbody chatbody_srcid_"+src_chatid+" chatbody_dstid_"+dst_chatid+"' style='display: none;'>";
            chattabcont += "    <div class='col history_table'>";
            chattabcont += "        <div class='history_table_scroll' style='overflow: auto;'>";
            chattabcont += "            <div class='grid history_table_items'></div>";
            chattabcont += "        </div>";
            chattabcont += "    </div>";
            chattabcont += "    <div class='col controls_table'>";
            chattabcont += "        <img src='images/smile.gif'></img>";
            chattabcont += "        <img class='showHistory' src='images/history.gif'></img>";
            chattabcont += "    </div>";
            chattabcont += "</div>";
            chattabcont += "<div class='row chatbody chatbody_srcid_"+src_chatid+" chatbody_dstid_"+dst_chatid+"' style='display: none;'>";
            chattabcont += "    <div id='status_line' class='col status_line'></div>";
            chattabcont += "    <div class='col controls_table'/>";
            chattabcont += "</div>";
            chattabcont += "<div class='row chatbody chatbody_srcid_"+src_chatid+" chatbody_dstid_"+dst_chatid+"' style='display: none;'>"
            chattabcont += "   <div class='col input_area'>";
            chattabcont += "       <textarea id='input_area' class='input_area'></textarea>";
            chattabcont += "   </div>";
            chattabcont += "   <div class='col controls_table'/>";
            chattabcont += "</div>";
        return $(chattabcont);
    }

    self.refresh = function (){
        self.setSize(cur_width, cur_height);
        self.main_div.find("#input_area").attr("onKeyDown", cur_on_key_down);    
        self.main_div.find(".chatbody").find(".showHistory").attr("onClick", cur_on_show_history);
        // every time scroll history to end

        var div = self.main_div.find(".history_table_scroll").filter("div:visible");
        div.attr("scrollTop", div.attr("scrollHeight"));
        // set msg align
        div.find(".row").find("div:odd").css("width", "100%");
    }

    self.clearHistory = function (msg){
        var msg_id     = msg.getId(); 
        var src_chatid = msg.getSrcChatId();
        var dst_chatid = msg.getDstChatId();
        self.main_div.find(".chatbody_srcid_"+src_chatid).find(".history_table_items").empty();
    }

    self.delMesg = function (msg){
        var msg_id     = msg.getId(); 
        var src_chatid = msg.getSrcChatId();

        if (!msg_id){
            return;
        }
        if (!src_chatid){
            src_chatid = "0";
        }
        self.main_div.find(".chatbody_srcid_"+src_chatid).find(".history_table_items").find(".msg_id_"+msg_id).empty(); 
    }

    self.addMesg = function (msg){
        var msg_id     = msg.getId(); 
        var src_chatid = msg.getSrcChatId();
        var dst_chatid = msg.getDstChatId();

        if (!msg_id){
            msg_id = "0";
        }
        if (!src_chatid){
            src_chatid = "0";
        }
        if (!dst_chatid){
            dst_chatid = "0";
        }

//        if (src_chatid*1 > 0 &&
          if  (self.main_div.find(".chatTabsPanel").find(".chattabid_0").length)
        {
            //  alert("src_chatid: "+src_chatidًٌ+" dst_chatid: "+dst_chatid);
            var dst = self.main_div.find(".chatbody_dstid_0");
            dst.removeClass("chatbody_dstid_0");
            dst.addClass("chatbody_dstid_"+dst_chatid);
            var src = self.main_div.find(".chatbody_srcid_0");
            src.removeClass("chatbody_srcid_0");
            src.addClass("chatbody_srcid_"+src_chatid);
            var tab = self.main_div.find(".chatTabsPanel").find(".chattabid_0");
            tab.removeClass("chattabid_0");
            tab.addClass("chattabid_"+src_chatid);
        }

        if (dst_chatid*1 > 0){
            var old_chatbody_dstid = null;
            var elements = self.main_div.find(".chatbody_srcid_"+src_chatid);
            if (elements.length){
                var regexp = /(chatbody_dstid_\d+)/;
                var res = regexp.exec(elements[0].className);
                if (res && res[1]){
                    old_chatbody_dstid = res[1];
                }
            }
            if (old_chatbody_dstid){
                self.main_div.find(".chatbody_srcid_"+src_chatid).removeClass(old_chatbody_dstid);
            }
            self.main_div.find(".chatbody_srcid_"+src_chatid).addClass("chatbody_dstid_"+dst_chatid);
        }

        if (!msg.getBody().length){
            // empty message, may be it's event
            return;
        }

        if (!self.main_div.find(".chatTabsPanel").find(".chattabid_"+src_chatid).length){
            // add tab
            self.main_div.find(".chatTabsPanel").find("div:first").append(
                self.get_chattab_element(src_chatid, dst_chatid)
            );
            // add tab content
            self.main_div.append(
                self.get_chattab_cont_element(
                    src_chatid, dst_chatid
                )
            );
            self.main_div.find(".chattabid_"+src_chatid).css("background", "#abb0c0");
            // it's first tab, select it
            if (self.main_div.find(".chatTabsPanel").find(".chattab").length == 1){
                self.selectChatById(src_chatid);
            }
        }

        var html = getMsgLine(msg);
        self.main_div.find(".chatbody_srcid_"+src_chatid).find(".history_table_items").append($(html));
        self.refresh();
    }

    var cur_date    = new Date;
    var unixtime_ms = cur_date.getTime();
    var unixtime    = parseInt(unixtime_ms / 1000);
    var msg = msgClass();
    msg.setId("0");
    msg.setTime(unixtime);
    msg.setType("system");
    msg.setBody(" ");
    msg.setSrcChatId("0");
    msg.setDstChatId("0");
    self.addMesg(msg);

    self.setStatus = function (msg){
        var new_status  = $("<small>"+msg+"</small>");
        var status_line = self.main_div.find("#status_line");
        status_line.empty();
        status_line.append(new_status);
    }

    self.onShowHistory = function(code){
        cur_on_show_history = code;
    };

    self.onKeyDown = function(code){
        cur_on_key_down = code;
    };

    self.setSize(default_width, default_height);
    self.refresh();
    return self;
};

