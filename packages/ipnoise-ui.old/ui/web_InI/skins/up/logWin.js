function TemplateLogWindowClass()
{
    var self = AbstractClass();
    var main_div_text  = "";
        main_div_text += "<div class='logWin' style='overflow: auto;'>";
        main_div_text += "   <div class='grid'></div>";
        main_div_text += "</div>";

    self.main_div = $(main_div_text);
 
    var default_width  = "300px";
    var default_height = "200px";

    self.setSize = function (width, height){
        self.main_div.find(".logWin").css("width",      width);
        self.main_div.find(".logWin").css("height",     height);
        self.main_div.find(".logWin").css("max-height", height);
        self.main_div.find(".logWin").css("max-width",  width);
    };

    self.refresh = function (){
        // set msg align
        self.main_div.find(".logWin").find(".row").css("width", "100%");
    }

    var count = 0;

    self.addMesg = function (msg){
        var class_name = "";
        if (count%2){
            class_name = "logWinRow1";
        } else {
            class_name = "logWinRow2";      
        }
        var data = '<div class="row '+class_name+'">'
            +'<div class="col">'+msg.getTime()+'</div>'
            +'<div class="col">'+msg.getBody()+'</div>'
            +'</div>';
       
        self.main_div.find(".grid").append($(data));
        count++;
    }

    self.setSize(default_width, default_height);
    self.refresh();
    return self;
};

