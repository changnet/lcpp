#include "lclass.hpp"
#include <iostream>
#include <sol/sol.hpp>

class Test
{
public:
    enum
    {
        V1 = 2
    };
    Test(): _i(0), _s("")
    {
    }

    int get_i() const
    {
        return _i;
    }

    void foo() const
    {
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
    std::string test_param(bool a, int b, float c, double d, const char* e, std::string f, void* g, std::string& h, char *i)
    {
        std::cout << "test_param" << std::endl
            << "    " << a << std::endl
            << "    " << b << std::endl
            << "    " << c << std::endl
            << "    " << d << std::endl
            << "    " << (e ? e : "") << std::endl
            << "    " << f << std::endl
            << "    " << g << std::endl
            << "    " << h << std::endl
            << "    " << (i ? i : "") << std::endl
            ;

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
private:
    int _i;
    std::string _s;
};

class TestCtor2
{
public:
    static TestCtor2 *instance()
    {
        if (!_ins) _ins = new TestCtor2();

        return _ins;
    }
    void dump()
    {
        std::cout << "dump TestCtor2" << std::endl;
    }
private:
    TestCtor2()
    {
    }
    static TestCtor2* _ins;
};
TestCtor2* TestCtor2::_ins = nullptr;

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

void test2(std::string s)
{
}

class player
{
public:
    void shoot(std::string &s)
    {
        std::cout << "shoot " << s << std::endl;
    }
};

void test3(std::string& i)
{
    std::cout << "test3 " << i << std::endl;
}

std::string fc_ret()
{
    return "";
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
    lt.def<&Test::foo>("foo");
    lt.def<&Test::set>("set");
    lt.def<&Test::sss>("sss");
    lt.def<&Test::test_param>("test_param");
    lt.set(Test::V1, "V1");

    LClass<TestCtor> ltc(L, "TestCtor");
    ltc.constructor<int, const char*>();
    ltc.def<&TestCtor::dump>("dump");

    LClass<TestCtor2> ltc2(L, "TestCtor2");
    ltc2.def<&TestCtor2::dump>("dump");

    Test gt;
    gt.set_i(111111);
    gt.set_s("222222222222");
    LClass<Test>::push(L, &gt); // lt.push(L, &gt)
    lua_setglobal(L, "gt");

    TestCtor2* gtc2 = TestCtor2::instance();

    LClass<TestCtor2> ltc2_1(L);
    ltc2_1.push_global(L, gtc2, "gtc2");

    luaL_dofile(L, "test.lua");
 
    lua_close(L);

    sol::state lua;
    lua.open_libraries(sol::lib::base);

    // test with sol

    int x = 0;
    lua.set_function("beep", [&x] { ++x; });
    lua.set_function("test2", test2);
    lua.script("beep()");
    assert(x == 1);

    sol::usertype<player> player_type = lua.new_usertype<player>("player",
        sol::constructors<player()>());
    player_type["shoot"] = &player::shoot;

    test3(fc_ret());

    return 0;
}
