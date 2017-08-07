var name    = 'global';
var deep    = 0;

function test()
{
    alert('name:   '+this.name);
    alert('deep:   '+this.deep);

    this.show_info = test;
}

var object1 = {
    name:       'object1',
    deep:       1,
    show_info:  test
};

var object2 = {
    name:       'object2',
    deep:       1,
    show_info:  test
};

object1.show_info();
object2.show_info();
test();

alert('-----------');
var a = new test();
a.name = 'object test';
a.deep = 2;
a.show_info();

