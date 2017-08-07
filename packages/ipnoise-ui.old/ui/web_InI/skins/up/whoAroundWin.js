function TemplateWhoAroundWindowClass(args)
{
    var self = AbstractClass(); 

    var default_width   = "300px";
    var default_height  = "200px";
   
    var cur_width       = default_width;
    var cur_height      = default_height;
 
    var main_div_text  = '<div class="whoAroundWin">';
        main_div_text += '                                      \
            <div class="grid">                                  \
            <div class="row">                                   \
                <div class="col">                               \
                    <a href="#" class="doSearch">Обновить</a>   \
                </div>                                          \
            </div>                                              \
            </div>                                              \
            <div class="searchRes grid">                        \
            </div>                                              \
        ';
        main_div_text += "</div>";
 
    self.main_div = $(main_div_text);
 
    // start search
    self.doSearch = null;

    self.setSize = function (width, height){
        cur_width  = width;
        cur_height = height;

        self.main_div.css("overflow",   "auto"); 
        self.main_div.css("width",      cur_width);
        self.main_div.css("height",     cur_height);
    };

    self.refresh = function (){
        self.setSize(cur_width, cur_height);
        self.main_div.find(".doSearch").click(function(){
            if (self.doSearch){
                self.doSearch();
            }
        });
    };

    self.setResults = function(items){
        self.main_div.find(".searchRes").empty();
        for (var i = 0; i < items.length; i++){
            var item = items[i];
            var huid        = item.getHuid();
            var name        = item.getName();
            var status_icon = item.getStatusIcon();
            var avatar_icon = item.getAvatarIcon();
            if (!avatar_icon || !avatar_icon.length || !/\w+/.test(avatar_icon)){
                avatar_icon = "images/no_photo.jpg";
            }
            var info = "";
            info += '<div class="grid">';
            info += '   <div class="row">';
            info += '       <div class="col">Name:</div>';
            info += '       <div class="col">'+name+'</div>';
            info += '   </div>';
            info += '   <div class="row">';
            info += '       <div class="col">Status:</div>';
            info += '       <div class="col"><img src="'+status_icon+'"></img></div>';
            info += '   </div>'; 
            info += '</div>';

            var class_name;
            if (i%2 == 0){
                class_name = "whoAroundWinRow2";
            } else {
                class_name = "whoAroundWinRow1";           
            }
            var html = "";
            html += '<div class="row item '+class_name+'"'
                            +' huid="'+huid+'"'
                            +'>';
            html += '    <div class="col avatar"><img src="'
                            +avatar_icon+'"></img></div>';
            html += '    <div class="col info" style="width: 100%;">'
                            +info+'</div>'; 
            html += '</div>';

            self.main_div.find(".searchRes").append($(html));
        }
        self.refresh();
    }

    self.refresh();
    return self;
};

