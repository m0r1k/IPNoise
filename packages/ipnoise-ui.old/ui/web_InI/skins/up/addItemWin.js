function TemplateAddItemWindowClass(args)
{
    var self = AbstractClass(); 

    var default_width   = "300px";
    var default_height  = "200px";
   
    var cur_width       = default_width;
    var cur_height      = default_height;
 
    var main_div_text  = '<div class="addItemWin">';
        main_div_text += '                                      \
            <div class="grid">                                  \
            <div class="row">                                   \
                <div class="col">                               \
                    Имя заметки:                                \
                </div>                                          \
                <div class="col" style="width: 20px"/>          \
                <div class="col">                               \
                    <input class="_ItemName" type="text" size="30"> \
                    </input>                                    \
                </div>                                          \
            </div>                                              \
            <div class="row">                                   \
                <div class="col">                               \
                    Описание:                                   \
                </div>                                          \
                <div class="col" style="width: 20px"/>          \
                <div class="col">                               \
                    <textarea class="_ItemDescr"></textarea>    \
                </div>                                          \
            </div>                                              \
            </div>                                              \
            <div class="row">                                   \
                <div class="col">                               \
                    <a class="_ItemSubmit">Добавить</a>         \
                </div>                                          \
            </div>                                              \
        ';
        main_div_text += "</div>";
 
    self.main_div = $(main_div_text);
 
    self.setSize = function (width, height){
        cur_width  = width;
        cur_height = height;

        self.main_div.css("width",  cur_width);
        self.main_div.css("height", cur_height);
    };

    self.onSubmit = null;

    self.refresh = function (){
        self.main_div.find("._ItemSubmit").click(
            function(){
                var div   = self.main_div;
                var name  = div.find("._ItemName").attr("value");
                var descr = div.find("._ItemDescr").attr("value");
                if (self.onSubmit){
                    self.onSubmit({
                        name:   name,
                        descr:  descr,
                    });
                }
            }
        );
    };
 
//    self.setSize(default_width, default_height);
    self.refresh();
    return self;
};

