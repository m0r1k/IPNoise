function TemplateHistoryWindowClass(args)
{
    var self = AbstractClass(); 

    var default_width   = "300px";
    var default_height  = "200px";
   
    var cur_width       = default_width;
    var cur_height      = default_height;
 
    // function for get history (must be installed by historyWindowClass)
    self.getHistory = null;

    if (args["getHistory"]){
        self.getHistory = args["getHistory"];
    }

    var main_div_text  = '<div class="historyWin">';
        main_div_text += '                                      \
            <div class="grid">                                  \
            <div class="row">                                   \
                <div class="col">                               \
                    Искать:                                     \
                </div>                                          \
                <div class="col" style="min-width: 20px"/>      \
                <div class="col" style="width: 100%">           \
                    <input class="searchText" type="text" size="30"/>\
                </div>                                          \
                <div class="col">                               \
                    <select class="searchIN">                   \
                        <option>В сообщениях</optіon>           \
                        <option>В ссылках</optіon>              \
                        <option>В тегах</optіon>                \
                    </select>                                   \
                </div>                                          \
            </div>                                              \
            </div>                                              \
            <div class="grid">                                  \
            <div class="row">                                   \
                <div class="col">                               \
                    <img width="10px" src="images/expandbtn2.gif"></img> \
                </div>                                          \
                <div class="col" style="width: 20px;" />        \
                <div class="col">                               \
                    Дополнительно                               \
                </div>                                          \
            </div>                                              \
            <div class="row">                                   \
                <div class="col" />                             \
                <div class="col" />                             \
                <div class="col"  >                             \
                   <ol class="message_list"/>                   \
                </div>                                          \
            </div>                                              \
            <div class="row">                                   \
                <div class="col" />                             \
                <div class="col" />                             \
                <div class="col"  >                             \
                    <p class="collapse_buttons">                \
                        <a href="#" class="show_all_message"></a>   \
                        <a href="#" class="show_recent_only">Show 5 only</a>            \
                        <a href="#" class="collpase_all_message">Collapse all</a>       \
                    </p>    \
                </div>      \
            </div>          \
            </div>          \
        ';
        main_div_text += "</div>";

    self.main_div = $(main_div_text);

    // request history 
    self.getHistory({});

    self.setSize = function (width, height){
        cur_width  = width;
        cur_height = height;

        self.main_div.css("width",  cur_width);
        self.main_div.css("height", cur_height);

        self.main_div.find(".message_list").css("overflow",  "auto");
        self.main_div.find(".message_list").css("width",     cur_width  - 50);
        self.main_div.find(".message_list").css("height",    cur_height - 100);
    };

    function loadChatHistory (chatid){
        if (self.main_div.find(".chatid_"+chatid).find(".history_table").length){
            self.main_div.find(".chatid_"+chatid).find(".message_body").slideToggle(500);
            return;
        }
        // request history 
        self.getHistory({
            "chatid":           chatid,
            "msg":              old_search_value,
        });
    }

    var first_search = 1;
    var old_search_value = "";
    self.refresh = function (){
        self.setSize(cur_width, cur_height);

        //hide message_body after the first one
        // self.main_div.find(".message_list .message_body:gt(0)").hide();
        self.main_div.find(".message_list .message_body").hide();
               
        //toggle message_body
        self.main_div.find(".message_head").click(function(){
            var className   = $(this.parentNode).attr("className");
            var chatid      = 0;
            var regexp      = /chatid_(\d+)/;
            var res = regexp.exec(className);
            if (!res || !res[1]){
                self.error("Cannot get chatid..");
                return;
            }
            chatid = res[1];
            loadChatHistory(chatid);
            return false;
        });

        //collapse all messages
        self.main_div.find(".collpase_all_message").click(function(){
            $(".message_body").slideUp(500)
            return false;
        });

        //show all messages
        self.main_div.find(".show_all_message").click(function(){
            $(this).hide()
            $(".show_recent_only").show()
            $(".message_list li:gt(4)").slideDown()
            return false;
        });

        //show recent messages only
        self.main_div.find(".show_recent_only").click(function(){
            $(this).hide()
            $(".show_all_message").show()
            $(".message_list li:gt(4)").slideUp()
            return false;
        });
        
        // input
        self.main_div.find(".searchText").keyup(function(){
            var value = $(this).attr("value");
            if (old_search_value != value){
                old_search_value = value;
                // request history
                self.getHistory({
                    "msg":   old_search_value
                });
            }
            return true;
        });
    };
 
    self.setHistory = function (chats)
    {
        // clear list
        self.main_div.find(".message_list").empty();

        var cur_date    = new Date;
        var unixtime_ms = cur_date.getTime();
        var cur_time    = parseInt(unixtime_ms / 1000);

        var open_chatid = null;
        for (var i = 0; i < chats.length; i++){
            var chat_info   = chats[i];
            var create_time = chat_info["create_date"];

            var chat_create_date = new Date(create_time * 1000);
            var year  = chat_create_date.getFullYear();
            var month = chat_create_date.getMonth();
            var day   = chat_create_date.getDate();
            var hour  = chat_create_date.getHours();
            var min   = chat_create_date.getMinutes();
            var sec   = chat_create_date.getSeconds();
            var msec  = chat_create_date.getMilliseconds();

            year  = text_int_align(year,  4);
            month = text_int_align(month, 2);
            day   = text_int_align(day,   2);
            hour  = text_int_align(hour,  2);
            min   = text_int_align(min,   2);
            sec   = text_int_align(sec,   2);
            msec  = text_int_align(msec,  3);

            var create_text = "";
            if (!create_time){
                create_time = 0;
                create_text = "";
            } else {
                var ago = cur_time - create_time;
                if (ago < 60){
                    create_text = ago+" сек. назад";
                } else if (ago < 3600){
                    ago = parseInt(ago/60);
                    create_text = ago+" мин. назад"
                } else if (ago < 3600*24){
                    ago = parseInt(ago/3600);
                    create_text = ago+" час. назад"
                } else {
                   create_text = hour+":"+min+" "+day+"/"+month+"/"+year;
                }
            }

            var html  = "";
                html += "<li class='chatid_"+chat_info.id+"'>";
                html += "   <p class='message_head'>";
                html += "      <cite>Чат</cite>";
                html += "      <span class='timestampMsgCount'>Сообщений: "+chat_info.msg_count+"</span>"; 
                html += "      <span class='timestamp'>"+create_text+"</span>";
                html += "   </p>";
                html += "   <div  class='message_body'>";

            if (chat_info.messages.length){
                html += "   <p>";
                html += "   <div class='grid history_table'>"; 
                for (var m = 0; m < chat_info.messages.length; m++){
                    var msg_obj = chat_info.messages[m];
                    html += getMsgLine(msg_obj);
                }
                html += "   </div>"; 
                html += "   </p>";
            }
            html += "   </div>";
            html += "</li>";

            self.main_div.find(".message_list").append($(html));
        }
        self.main_div.find(".show_all_message").text("Show all message ("+chats.length+")");
        self.refresh();
        for (var i = 0; i < chats.length; i++){
            var chat_info = chats[i];
            if (chat_info.messages.length){
                self.main_div.find(".chatid_"+chat_info.id).find(".message_body").slideToggle(500);
            }
        }
        if (first_search){
            // hide message li after the 5th
            self.main_div.find(".message_list li:gt(5)").hide();
            first_search = 0;
        } 
    }

    self.refresh();
    return self;
};

