// hi there

//var tests = {};

/*
function context_global_func()
{
    alert(this);
};

tests.context = function()
{
    var ret = true;

    var abc = {
        tigra:      3,
        array_func: function(){
            alert(this);
        }
    };

    context_global_func();
    abc.array_func();

    return ret;
};

tests.additional = function()
{
    var err = true;
    var a   = -1.123456789;
    var b   = 1.987654321;
    var c   = a + b;

    if (0.8641975319999999 != c){
        err = false;
    }

    return err;
};

tests.subtraction = function()
{
    var err = true;
    var a   = -1.123456789;
    var b   = 1.987654321;
    var c   = a - b;

    if (-3.11111111 != c){
        err = false;
    }

    return err;
};

tests.multiplication = function()
{
    var err = true;
    var a   = -1.123456789;
    var b   = 1.987654321;
    var c;

    c = a * b;
    if (-2.233043741112635 != c){
        err = false;
    }

    c = b * a;
    if (-2.233043741112635 != c){
        err = false;
    }

    if (-2.233043741112635 != (a * b)){
        err = false;
    }

    return err;
};

tests.logical_or = function()
{
    var err   = true;
    var res;

    var code = function(a, b){
        return a * b;
    };
    var code_echo = function(a){
        return a;
    };

    // case 1
    res = code(0, 3)
        || code(2, 0)
        || code(0, 0)
        || code_echo(1);

    if (res != true){
        err = false;
    };

    // case 2
    res = code(0, 3)
        || code(2, 0)
        || code(0, 0)
        || code_echo(0);

    if (res == true){
        err = false;
    };

    // case 3
    res = code(0, 3)
        || code(2, 0)
        || code(3, 7)
        || code_echo(0);

    if (res != true){
        err = false;
    };

    return err;
};

tests.for_loop = function()
{
    var err = true;
    var res = "";

    for (var x = 0, y = 10;
        x < 10 && y < 20;
        x++, y++)
    {
        //res += x + y;
        alert(x+': '+y);
    };

    return err;
};

tests.timeout = function(a_parser)
{
    var err = true;

    var a = setTimeout(
        function(a_parser){
            alert('timeout after 2 sec');
            a_parser.exit();
        },
        2000,
        a_parser
    );
    //var b = setTimeout(
    //    function(a_parser){
    //        alert("timeout after 5 sec");
    //    },
    //    5000,
    //    a_parser
    //);
    //var c = setTimeout(
    //    function(a_parser){
    //        alert("timeout after 15 sec");
    //        a_parser.exit(0);
    //    },
    //    15000,
    //    a_parser
    //);

    alert(a);
    //alert(b);
    //alert(c);

    return err;
};
*/

/*
tests.events = function(a_parser)
{
    var err = true;

    a_parser.addEventListener("accept",
        function (ev){
            alert("new connection 1 " + ev.morik + "\n");
        };
    );
    a_parser.addEventListener("accept",
        function (ev){
            alert("new connection 2 " + ev.morik + "\n");
        };
    );
    a_parser.addEventListener("accept",
        function (ev){
            alert("new connection 3 " + ev.morik + "\n");
            ev.target.exit(0);
        };
    );

    var ev = new Action("accept");
    ev.target = a_parser;

    ev.morik = "tigra was here";
    a_parser.dispatchEvent(ev);

    ev.morik = "migra was here";
    a_parser.dispatchEvent(ev);

    return err;
};

tests.http = function(a_parser)
{
    var err = true;

    a_parser.listen("0.0.0.0:8080");
    a_parser.addEventListener(
        "accept",
        function (ev){
            alert("new accept: "+ev+"\n");
            //alert(a_parser);
            //var target = ev.target;
            //alert('target: '+target);
            //target.close();
        };
    );

    a_parser.addEventListener(
        "read",
        function (ev){
            alert("new read\n");
            var target = ev.target;
            //alert('target: '+target);
            target.close();
            a_parser.exit(0);
        };
    );

    a_parser.addEventListener(
        "event",
        function (ev){
            alert("new event\n");
        };
    );

    this.morik = 12;

    return err;
};
*/

//tests.context()         || alert("fail, context");
//tests.additional()      || alert("fail, additional");
//tests.subtraction()     || alert("fail, subtraction");
//tests.multiplication()  || alert("fail, multiplication");
//tests.logical_or()      || alert("fail, logical_or");
//tests.for_loop()        || alert("fail, for_loop");
//tests.timeout(this)     || alert("fail, timeout");
//tests.events(this)      || alert("fail, events");
//tests.http(this)        || alert("fail, http");

function morik(){
    alert("morik was here\n");
};

morik();
exit();

