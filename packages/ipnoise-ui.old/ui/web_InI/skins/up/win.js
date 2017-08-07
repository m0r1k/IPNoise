function TemplateWindowClass(args)
{
    var self = AbstractClass(); 
    var hide_head = 0;
    if (args["hide-head"]){
        hide_head = 1;
    }

    var main_div_text  = '<div class="win grid" id="'+args.id+'">';
        main_div_text += '                                                                                  \
                <div class="row">                                                                           \
                <div class="col">                                                                           \
                    <div class="grid winHead">                                                                  \
                        <div class="row">                                                                       \
                            <div class="col winTitle" style="width: 100%"></div>                                \
                            <div class="col minimizeWin"><img src="images/icon-minimize.gif"></img></div>       \
                            <div class="col maximizeWin"><img src="images/icon-maximize.gif"></img></div>       \
                            <div class="col closeWin"><img src="images/close-32.png"></img></div>               \
                        </div>                                                                                  \
                    </div>                                                                                      \
                </div>                                                                                      \
                </div>                                                                                      \
                <div class="row">                                                                           \
                <div class="body col" style="overflow: auto;"></div>                                            \
                </div>                                                                                      \
        ';
        main_div_text += "</div>";
 
    self.main_div = $(main_div_text);

    var default_width  = "300px";
    var default_height = "200px";
 
    if (hide_head){
        self.main_div.find(".winHead").css("display", "none");
    }

    self.minimize = function (){
        self.main_div.find(".body").hide();//css("height", "0px");
    }

    self.maximize = function (){
        self.main_div.find(".body").show();//css("height", "");
    }

    self.setSize = function (width, height){
        self.main_div.css("max-width",  width+"px");
        self.main_div.css("max-height", height+"px");
        self.main_div.find(".body").css("max-height", (height - 10)+"px"); 
        self.main_div.find(".body").css("max-width",  (width - 10)+"px");  
    };

    self.setFixedSize = function (width, height){
        self.main_div.css("width",  width+"px");
        self.main_div.css("height", height+"px");
        self.main_div.find(".body").css("height", (height - 10)+"px"); 
        self.main_div.find(".body").css("width",  (width - 10)+"px");  
    };

    self.refresh = function (){
    }

    self.setBody = function (new_body){
        var body = self.main_div.find(".body");
        body.empty();
        body.append(new_body);
    }

    self.setTitle = function (title){
        self.main_div.find(".winTitle").text(title);
    }

    self.onClose = function(code){
        self.main_div.find(".winHead").find(".closeWin").attr("onClick", code);
    };

    self.onMinimize = function(code){
        self.main_div.find(".winHead").find(".minimizeWin").attr("onClick", code);
    };

    self.onMaximize = function(code){
        self.main_div.find(".winHead").find(".maximizeWin").attr("onClick", code);
    };

    self.setSize(default_width, default_height);
    self.refresh();
    return self;
};

