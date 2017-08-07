function msgClass()
{
    var self = AbstractClass();

    var type = null;
    var body = null;
    var time = null;

    self.isIncoming = function(){
        var ret;
        if (   type == "incoming"
            || type == "system")
        {
            ret = 1;
        } else {
            ret = 0;
        }
        return ret;
    }

    self.getType = function(){
        return type;
    }

    self.setType = function(new_type){
        if (   new_type != "incoming"
            && new_type != "outcoming"
            && new_type != "system")
        {
            perror("Invalid message type: '"+new_type+"'");
            return null;
        }
        type = new_type;
    }

    self.getTime = function(){
        return time;
    }

    self.setTime = function(new_time){
        time = new_time;
    }

    self.setBody = function(new_body){
        body = new_body;
    }

    self.getBody = function(){
        return body;
    }

    return self;
};

function TemplateMessageWindowClass()
{
    var self = AbstractClass();
    var main_div_text = ''+<>
            <div class="msgWin grid">
            <div class="row">
                <div class="col input_area">
                    <textarea id="input_area" style="width: 100%;"></textarea>
                </div>
            </div>
            <div class="row">
               <div id="status_line" class="col" style="height: 20px;"></div>
            </div>
            <div class="row">
                <div class="col">
                    <img src="images/smile.gif"></img>
                </div>
            </div>
            <div class="row">
                <div class="col history_table">
                    <div class="history_table_scroll" style="overflow: auto;">
                        <div class="grid history_table_items"></div>
                    </div>
                </div>
            </div>
            </div>
        </>;
    var main_div = $(main_div_text);
 
    var default_width  = "300px";
    var default_height = "200px";
    
    self.getElement = function (){
        return main_div;
    };

    self.setSize = function (width, height){
        main_div.find(".msgWin").css("width",  width);
        main_div.find(".msgWin").css("height", height);
    
        main_div.find(".history_table").css("width", width);
        main_div.find(".history_table_scroll").css("height", height);

        var input_height = "100px";
        main_div.find("#input_area").css("height", input_height);
    };

    self.refresh = function (){
        // every time scroll history to end
        var div = main_div.find(".history_table_scroll");
        div.attr("scrollTop", 0); //div.attr("scrollHeight"));

        // set msg align
        main_div.find(".history_table_scroll").find(".row").find("div:odd").css("width", "100%");
    }

    self.addMesg = function (msg){
        var class_name = "";
        if (msg.getType() == "outcoming"){
            class_name = "msgOutcoming";
        } else if (msg.getType() == "incoming"){
            class_name = "msgIncoming";
        } else if (msg.getType() == "system"){
            class_name = "msgSystem";
        } else {
            class_name = "msgUnknown";
        }
        var msg = $('<div class="row '+class_name+'">'
            +'<div class="col">'+msg.getTime()+'</div>'
            +'<div class="col">'+msg.getBody()+'</div>'
            +'</div>');
        $(".history_table_items").prepend(msg);
    }

    self.setStatus = function (msg){
        var new_status  = $("<small>"+msg+"</small>");
        var status_line = main_div.find("#status_line");
        status_line.empty();
        status_line.append(new_status);
    }

    self.setSize(default_width, default_height);

    self.refresh();
    return self;
};

