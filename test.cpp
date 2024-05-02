#include "lclass.hpp"
#include <iostream>

class Test {
public:
    Test() {}

    int get_i() {
        return _i;
    }

    void set_i(int i) {
        _i = i;
    }

    const char *get_s() {
        return _s.c_str();
    }

    void set_s(const char *i) {
        _s = i;
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
    std::cout << t->get_i() << "    " << t->get_s() << std::endl;
}

void test()
{
    std::cout << "test ==============" << std::endl;
}

int test1(lua_State* L)
{
    std::cout << "test1" << lua_tonumber(L, 1) << "    " << lua_tostring(L, 2) << std::endl;
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

    luaL_dofile(L, "test.lua");
 
    lua_close(L);
    return 0;
}
