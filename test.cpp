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

Test *newTest() {
    auto p = new Test;
    p->set_i(22222);
    p->set_s("afsadfdsfasf");
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

int main(int argc, char *argv[]) {
    auto L = luaL_newstate();
    luaL_openlibs(L);

    // register global function
    reg_global_func<&newTest>(L, "newTest");
    reg_global_func<&dumpTest>(L, "dumpTest");
    reg_global_func<&test>(L, "test");
    reg_global_func<&test1>(L, "test1");

    LClass<Test> lt(L, "Test");
    lt.def<&Test::set_i>("set_i");
    lt.def<&Test::set_s>("set_s");
    lt.def<&Test::get_i>("get_i");
    lt.def<&Test::get_s>("get_s");
    lt.def<&Test::set>("set");
    lt.def<&Test::sss>("sss");
    lt.def<&Test::test_param>("test_param");

    Test gt;
    gt.set_i(111111);
    gt.set_s("222222222222");
    lt.push(L, &gt);
    lua_setglobal(L, "gt");

    luaL_dofile(L, "test.lua");
 
    lua_close(L);
    return 0;
}
