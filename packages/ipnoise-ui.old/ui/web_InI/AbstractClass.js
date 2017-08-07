/*
 *  Roman E. Chechnev, (c) 2008-2009
 *  rchechnev%at%mail.ru
 *
 *  Abstract class implementation
 *  Sep, 2008.
 *
 *  Last change: Jun 01, 2009
 *
 *  Class dependence:
 *  Methods:
 *    pdebug
 *    pwarn
 *    perror
 *    pinfo
 */

var abstract_all_classes    = {};   // all classes, key - class ID
var abstract_class_id       = 1;    // start class id
var abstract_class_grid     = 1;    // start class group id

function AbstractClass(super_class_name)
{
    var self = this;

    // search caller class name
    var caller_class_func = AbstractClass.caller;
    // caller will return function content,
    // example:
    // function ParserClass() { 
    // .............
    // }
    // get function name
    var regexp = /function\s+([^\(]+)/;
    var res = regexp.exec(caller_class_func);
    if (!res || !res[1]){
        perror("Cannot get AbstractClass caller function name,\n"
            +"may be AbstractClass will called not from function?\n"
            +"AbstractClass.caller:\n'"+caller_class_func+"'"
        );
        return null;
    }

    var caller_class_name = res[1];
    if (! /Class\s*$/.test(caller_class_name) ){
        perror("AbstractClass caller function name,\n"
            +"must contain 'Class' text at end, example: 'BaseClass'\n"
            +"AbstractClass.caller:\n'"+caller_class_name+"'"
        );
        return null;
    }

    // Call base class if argument exist
    if (super_class_name && typeof super_class_name != "object"){
        // remove super class name from args
        var new_arguments = [];
        for (var i = 0; i < arguments.length; i++){
            if (i){
                // skip first argument
                new_arguments.push(arguments[i]);
            }
        } 

        var super_class_ref = eval (super_class_name+".apply({}, new_arguments);");
        if (!super_class_ref){
            perror("Cannot init class: '"+super_class_name+"'");
            return null;
        }
        // store what we inherited
        super_class_ref.abstract.inherited.push(super_class_name);
        // rewrite class name
        super_class_ref.abstract.class_name = caller_class_name;

        // copy super class methods to hash with super class name
        for (var super_attribute in super_class_ref){
            if (typeof super_class_ref[super_attribute] != "function"){
                continue;
            }
            if (!super_class_ref[super_class_name]){
                super_class_ref[super_class_name] = {};
            }
            super_class_ref[super_class_name][super_attribute] = super_class_ref[super_attribute];
        }
        // now SUPER attribute will contain our class
        super_class_ref.SUPER = super_class_ref[super_class_name];
        return super_class_ref;
    }

    // create new AbstractClass object
    if (!arguments.length
        || !typeof AbstractClass.arguments[0] == "object"
        || !AbstractClass.arguments[0].abstract_top_class)
    {
        var new_class = new AbstractClass({abstract_top_class: 1});
        if (!new_class.abstract.class_name){
            new_class.abstract.class_name = caller_class_name;
        }
        // copy our Abstract methods to hash with class name (AbstractClass)
        for (var self_attribute in new_class){
            if (typeof new_class[self_attribute] != "function"){
                continue;
            }
            if (!new_class.AbstractClass){
                new_class.AbstractClass = {};
            }
            new_class.AbstractClass[self_attribute] = new_class[self_attribute];
        }
        return new_class;
    }

    //
    // Init Abstract class variables and method
    //

    // current global class count + 1
    abstract_class_id++;
    var class_id        = abstract_class_id - 1;
    // current global class_groups count + 1
    abstract_class_grid++;
    var class_group_id  = abstract_class_grid - 1;

    self.abstract = {
        class_id:       class_id,                   // unique class ID
        class_grid:     class_group_id,             // unique class group ID
        inherited:      ["AbstractClass"],
        class_deep:     0,                          // class deep
        class_name:     null,                       // class name
        child_classes:  [],                         // child classes
        class_parent:   null,                       // parent class
                                                    // (uses for class.new())
    };

    // store our class in global classes list
    abstract_all_classes[class_id] = self;

    // Function for create subclass
    self["new"] = function(){
        // get new class object
        var new_class  = eval(self.abstract.class_name+".apply({}, arguments);");
        // update class deep, copy class group ID, store parent class
        new_class.abstract.class_deep   = self.abstract.class_deep + 1;
        new_class.abstract.class_grid   = self.abstract.class_grid;
        new_class.abstract.class_parent = self;

        // copy public data in new class
        for (var prop in self.publicData){
            if (!new_class.publicData){
                new_class.publicData = {};
            }
            new_class.publicData[prop] = self.publicData[prop];
        }

        self.abstract.child_classes.push(new_class);
        return new_class;
    }
    // Function return class deep
    self.getClassDeep = function (){
        if (arguments.length){
            perror("Read-only method");
        }
        return self.abstract.class_deep;
    }
    // Function return class ID
    self.getClassId = function (){
        if (arguments.length){
            perror("Read-only method");
        }
        return self.abstract.class_id;
    }
    // Function return GUI class ID
    self.getClassGuiId = function (){
        if (arguments.length){
            perror("Read-only method");
        }
        return "InI_classid_"+self.abstract.class_id;
    }
    // Function return class group ID
    self.getClassGrid = function (){
        if (arguments.length){
            perror("Read-only method");
        }
        return self.abstract.class_grid;
    }
    // Function return parent class if exist
    self.getParent = function(){
        return self.abstract.class_parent;
    };
    // Function return only our childs
    self.getChilds = function(){
        return self.abstract.child_classes;
    };
    // Function delete child
    self.delChild = function(child){
        if (typeof child != "object"){
            perror("Argument must be object");
            return null;
        }
        var new_childs = [];
        var cur_childs = self.abstract.child_classes;
        for (var i = 0; i < cur_childs.length; i++){
            var cur_child = cur_childs[i];
            if (cur_child != child){
                new_childs.push(cur_child);
            }
        }
        self.abstract.child_classes = new_childs;
    };
    // Function return all our classes childs (recursive)
    self.getAllChilds = function(){
        var childs_res = [];
        for (var i = 0; i < self.abstract.child_classes.length; i++){
            var child_class = self.abstract.child_classes[i];
            childs_res = childs_res.concat(child_class.getAllChilds());
        }

        // concat our childs with childs_our_childs
        return childs_res.concat(self.abstract.child_classes);
    };
    // Function append new class in childs list
    // if it class have childs it deep will recalculated
    self.appendChild = function(class_ref){
        if (typeof class_ref != "object"){
            perror("Cannot append child class,"
                +" 'class_ref' argument is null or not object",
                class_ref
            );
            return null;
        }
        if (!class_ref.abstract.class_name){
            perror("Cannot append child class,"
                +" cannot get class name from abstract info"
            );
            return null;
        }
        var found = 0;
        if (self.abstract.class_name == class_ref.abstract.class_name){
            found = 1;
        } else {
            for (var i = 0; i < class_ref.abstract.inherited.length; i++){
                var class_name = class_ref.abstract.inherited[i];
                if (self.abstract.class_name == class_name){
                    found = 1;
                    break;
                }
            }
        }
        if (!found){
            perror("Cannot append child class,"
                +" child class have not contain parent class in inherited list"
                +" '"+self.abstract.class_name+"' != '"+class_ref.abstract.class_name+"'",
                "Child inherite class list: "+class_ref.abstract.inherited.join()
            );
            return null;
        }
        // looks like all ok, recalculate deeps
        var child_start_deep = self.abstract.class_deep + 1;
        class_ref.abstract.class_deep += child_start_deep;
        // store new parent class
        class_ref.abstract.class_parent = self;
        // recalculate childs class deeps if childs exist
        var childs = class_ref.getAllChilds();
        for (var i = 0; i < childs.length; i++){
            var child = childs[i];
            child.abstract.class_deep += child_start_deep;
            child.abstract.class_grid = self.abstract.class_grid;
        }
        // store class as our child
        self.abstract.child_classes.push(class_ref);
        return 1;
    };
    // Log functions
    self.debug = function (msg, detail){
        pdebug(msg, detail, self.abstract.class_id);
    };
    self.warn = function (msg, detail){
        pwarn(msg, detail, self.abstract.class_id);
    };
    self.error = function (msg, detail){
        perror(msg, detail, self.abstract.class_id);
    };
    self.info = function (msg, detail){
        pinfo(msg, detail, self.abstract.class_id);
    };

    // XML document to text convertor
    self.XML2Text = function (element){
        // unfortunate XMLSerializer return xml as one line
        // without formating.. do formating..

        var serializer = new XMLSerializer();
        var data = "";
        var stream = {
            close : function(){},
            flush : function(){},
            write : function(string, count){
                data += string;
            }
        };
        serializer.serializeToStream(element, stream, "UTF-8");

        data = data.replace(/></g, ">\n<");

        var lines = data.split("\n");
        var deep = -1;
        var res = "";
        for (var i = 0; i < lines.length; i++){
            var line = lines[i];
            var regexp_res;

            while ((regexp_res = /(<[^\/])/g.exec(line)) != null){
                deep++;
            }
            res += "\n";
            for (var m = 0; m < deep; m++){
                res += "    ";
            }
            res += line;

            while ((regexp_res = /(\/>)/g.exec(line)) != null){
                deep--;
            }
            while ((regexp_res = /(<\/)/g.exec(line)) != null){
                deep--;
            }
        }
        return res;
    };
};

function getAllClasses()
{
    return abstract_all_classes;
}

function getClassById (class_id){
    if (!arguments.length){
        perror("Missing argument: 'class_id'");
        return null;
    }
    if (!class_id){
        return null;
    }
    return abstract_all_classes[class_id];
}

function getClassByGuiId (gui_class_id)
{
    if (!gui_class_id){
        return null;
    }
    var res;
    if (res = /InI_classid_(\d+)/.exec(gui_class_id)){
        var class_id  = res[1];
        var class_ref = getClassById(class_id);
        if (class_ref){
            return class_ref;
        }
    }
    // perror("Cannot get class by GUI ID: '"+gui_class_id+"'");
    return null;
}

function cleanClassById(class_id)
{
    if (!class_id){
        return null;
    }
    delete (abstract_all_classes[class_id]);
    return 1;
};

function cleanClass(class_ref)
{
    if (typeof class_ref != "object"){
        return null;
    }
    // TODO REMOVE CHILS!!! FREE MEMORY
    var class_id = class_ref.getClassId();
    cleanClassById(class_id);
    class_ref = null;
    return 1;
};

/*

AbstractClass:

property:

publicData - it property value will copied in classes what created
             via new() method

1. How to create new Class?

    You should create class as function and use context from AbstractClass();
    NOTE: This function name must contain 'Class' text at end. (ex. BaseClass)

    Example:

    function SimpleClass ()
    {
        var self = AbstractClass();
        self.helloWorld = function (){
            alert("Hello World!");
        };
        // DO NOT FORGET RETURN SELF
        return self;
    }

    Create class exemplar and execute method:

    var class = SimpleClass();
    class.helloWorld();

2. How to inherit class from other class?

    You should say to AbstractClass, class name inherit from;
    NOTE: inherit class MUST use AbstractClass as context;

    Example:

    function VerySimpleClass ()
    {
        var self = AbstractClass("SimpleClass");
        self.helloWorld = function (){
            alert("Hello beautiful World!");
        };
        // DO NOT FORGET RETURN SELF
        return self;
    }

    VerySimpleClass will inherited from SimpleClass
    and have redefine helloWorld method.

    Create class exemplar and execute method:

    var class = VerySimpleClass();
    class.helloWorld();

3. How to call base class(es) method(s)?

    Your class object contain keys with base classes names, are keys
    contain base classes methods.

    Example:

    function SimpleClass ()
    {
        var self = AbstractClass();
        self.helloWorld = function (){
            alert("Hello World!");
        };
        // DO NOT FORGET RETURN SELF
        return self;
    }

    function VerySimpleClass ()
    {
        var self = AbstractClass("SimpleClass");
        self.helloWorld = function (){
            alert("Hello beautiful World!");
        };
        // DO NOT FORGET RETURN SELF
        return self;
    }

    var class = VerySimpleClass();
    class.helloWorld();

    if run you will see "Hello beautiful World!" alert,
    to call base class method "helloWorld" you can use
    next code:

    class.SimpleClass.helloWorld();

    after run you will see "Hello World!" alert.

4. Final example

    // AbstractBackEnd Class inherited from AbstractClass class
    // and have defined public:
    //
    // 1. url       property
    // 2. showUrl   method
    // 
    function AbstractBackEndClass ()
    {    
        var self = AbstractClass();
        
        self.url     = "http://google.com";
        self.showUrl = function () {
            alert("AbstractBackEnd class, method 'showUrl()', self.url: "+self.url);
        };
        // DO NOT FORGET RETURN SELF
        return self;
    };

    //
    // BackEnd Class inherited from AbstractBackEnd class
    // and have added one public property 'debug'
    // and have redefined 'showUrl' public method
    // 
    function BackEndClass ()
    {    
        var self = AbstractClass("AbstractBackEndClass");
        self.debug = "enable";
        self.showUrl = function () {
            alert("BackEnd class, method 'showUrl()', self.url: "+self.url);
        };
        // DO NOT FORGET RETURN SELF
        return self;
    };

    //
    // PerlBackEnd Class inherited from BackEnd class
    // and have added one public method 'showDebugState()';
    // 
    function PerlBackEndClass ()
    {    
        var self = AbstractClass("BackEndClass");
        self.showDebugState = function () {
            alert("Debug state: '"+self.debug+"'");
        };
        // DO NOT FORGET RETURN SELF
        return self;
    };

    // create class object
    var classref = PerlBackEnd();
    classref.id();
    classref.debug("debug");

4. End of help

*/

