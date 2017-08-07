function N900()
{
    var ret = {
        'getCameraSnapshot':    function(){ alert('hello world'); },
        'name':                 "my phone",
        'battery_level':        77
    };

    return ret;
}

var phone = N900();

//phone.getCameraSnapshot();

alert ( phone.name );

