# lcpp
test my idea about lua cpp binding, this repo will not update after test finish!

# usage
```cpp
#include <lclass.hpp>

void func1(int a, int b, std::string c)
{
}

class Test1
{
public:
    enum
    {
        V1 = 2
    }
    Test1() {}
    Test1(int a, int b) {}
    void t1_func1(int a, int b, std::string c) {}
}

class Test2
{
public:
    static Test2 *instance()
    {
        if (!_ins) _ins = new Test2();

        return _ins;
    }
    void t2_func1(int a, int b, std::string c) {}
private:
    Test2()
    {
    }
    static Test2* _ins;
};
Test2* Test2::_ins = nullptr;

lclass::reg_global_func<&func1>("func1");

LClass<Test1> lt1(L, "Test1");
// or LClass<Test1, int, int> lt1(L, "Test1"); // using 2nd constructor
lt1.def<&Test1::t1_func1>("t1_func1");
lt1.set<Test1::V1, "V1");

LClass<Test2> lt2(L, "Test2");

Test2 *t2 = Test2::instance();
lt2.push_global(L, t2, "gt2");
```

```Lua
func1();

local Test1 = require "Test1"
local t1 = Test1(111, 222)
t1:t1_func1();


gt2:t2_func1()
```