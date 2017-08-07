function TemplateMainWindowClass(args)
{
    var self = AbstractClass(); 

    var default_width   = "300px";
    var default_height  = "200px";
   
    var cur_width       = default_width;
    var cur_height      = default_height;
 
    var main_div_text  = '<div class="mainWin">';
        main_div_text += '                                      \
            <div class="mainWinControls grid">                  \
            <div class="row">                                   \
                <div class="col">                               \
                    <img class="whoAround"                      \
                        src="images/search-icon.jpg"            \
                        title="Поиск пользователей">            \
                    </img>                                      \
                </div>                                          \
            </div>                                              \
            </div>                                              \
            <div class="contactList"/>                          \
        ';
        main_div_text += "</div>";
 
    self.main_div = $(main_div_text);
 
    self.setSize = function (width, height){
        cur_width  = width;
        cur_height = height;

        self.main_div.css("width",  cur_width);
        self.main_div.css("height", cur_height);
    };

    self.onWhoAround = null;

    self.refresh = function (){
        self.main_div.find(".whoAround").click(function(){
            if (self.onWhoAround){
                self.onWhoAround();
            } 
        });
    };
 
    self.refresh();
    return self;
};

