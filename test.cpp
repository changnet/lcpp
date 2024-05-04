#include "lclass.hpp"
#include <iostream>

class Test {
public:
    Test(): _i(0), _s("")
    {
    }

    int get_i()
    {
        return _i;
    }

    void set_i(int i)
    {
        _i = i;
    }

    const char *get_s()
    {
        return _s.c_str();
    }

    void set_s(const char *i)
    {
        _s = i;
    }

    int set(lua_State* L)
    {
        // param stack index begin from 2
        _i = (int)lua_tointeger(L, 2);
        _s = lua_tostring(L, 3);

        lua_pushboolean(L, true);
        return 1;
    }

    // only std::string, no std::string &
    std::string test_param(bool a, int b, float c, double d, const char* e, std::string f, void* g)
    {
        std::cout << "test_param" << std::endl
            << "    " << a << std::endl
            << "    " << b << std::endl
            << "    " << c << std::endl
            << "    " << d << std::endl
            << "    " << (e ? e : "") << std::endl
            << "    " << f << std::endl
            << "    " << g << std::endl;

        return "test param return something!!!";
    }

    static int sss(lua_State* L)
    {
        std::cout << "a class static method" << std::endl;
        return 0;
    }
private:
    int _i;
    std::string _s;
};

class TestCtor
{
public:
    TestCtor()
    {
        _i = 0;
        _s = "";
    }

    TestCtor(int i, const char* s): _i(i), _s(s)
    {
    }
    void dump()
    {
        std::cout << "dump TestCtor" << std::endl
            << "    " << _i << std::endl
            << "    " << _s << std::endl;
    }
    static TestCtor& instance()
    {
        return *_ins;
    }
private:
    int _i;
    std::string _s;
    static TestCtor* _ins;
};
TestCtor* TestCtor::_ins = nullptr;

class TestCtor2
{
private:
    TestCtor2()
    {
    }
};

Test *newTest() {
    auto p = new Test;
    return p;
}

void dumpTest(Test *t) {
    std::cout << "dump: " << t->get_i() << "    " << t->get_s() << std::endl;
}

void test()
{
    std::cout << "test func run" << std::endl;
}

int test1(lua_State* L)
{
    std::cout << "test1    " << lua_tonumber(L, 1) << "    " << lua_tostring(L, 2) << std::endl;
    lua_pushinteger(L, 44525234);
    return 1;
}

int main(int argc, char *argv[])
{
    auto L = luaL_newstate();
    luaL_openlibs(L);

    lclass::reg_global_func<&newTest>(L, "newTest");
    lclass::reg_global_func<&dumpTest>(L, "dumpTest");
    lclass::reg_global_func<&test>(L, "test");
    lclass::reg_global_func<&test1>(L, "test1");

    LClass<Test> lt(L, "Test");
    lt.def<&Test::set_i>("set_i");
    lt.def<&Test::set_s>("set_s");
    lt.def<&Test::get_i>("get_i");
    lt.def<&Test::get_s>("get_s");
    lt.def<&Test::set>("set");
    lt.def<&Test::sss>("sss");
    lt.def<&Test::test_param>("test_param");

    LClass<TestCtor, int, const char*> ltc(L, "TestCtor");
    ltc.def<&TestCtor::dump>("dump");

    //LClass<TestCtor2> ltc2(L, "TestCtor2");

    Test gt;
    gt.set_i(111111);
    gt.set_s("222222222222");
    lt.push(L, &gt);
    lua_setglobal(L, "gt");

    luaL_dofile(L, "test.lua");
 
    lua_close(L);
    return 0;
}
