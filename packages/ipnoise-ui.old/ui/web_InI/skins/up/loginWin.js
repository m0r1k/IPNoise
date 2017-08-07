function TemplateLoginWindowClass()
{
    var self = AbstractClass();
    var main_div_text = "";
        main_div_text += "<div class='grid loginWindow'>";
        main_div_text += "    <div class='row'>";
        main_div_text += "        <div class='col'>Login:    </div>";
        main_div_text += "        <div class='col'>";
        main_div_text += "            <input class='login' type='text'></input>";
        main_div_text += "        </div>";
        main_div_text += "    </div>";
        main_div_text += "    <div class='row'>";
        main_div_text += "        <div class='col'>Password: </div>";
        main_div_text += "        <div class='col'>";
        main_div_text += "            <input class='password' type='password'></input>";
        main_div_text += "        </div>";
        main_div_text += "    </div>";
        main_div_text += "    <div class='row'>";
        main_div_text += "        <div class='col' colspan='2'>";
        main_div_text += "            <a class='do_login'>login</a>";
        main_div_text += "         </div>";
        main_div_text += "    </div>";
        main_div_text += "</div>";

    self.main_div = $(main_div_text);
 
    var default_width  = "300px";
    var default_height = "200px";
    
    self.setSize = function (width, height){
        self.main_div.find(".msgWin").css("width",      width);
        self.main_div.find(".msgWin").css("height",     height);
    };

    self.refresh = function (){
    }

    self.onKeyDown = function(msg){
        self.main_div.find(".password").attr("onKeyDown", msg);
    };

    self.onLogin = function(msg){
        self.main_div.find(".do_login").attr("onclick", msg);
    };

    self.setSize(default_width, default_height);
    self.refresh();
    return self;
};

