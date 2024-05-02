#pragma once

#include <lua.hpp>
#include <string>

template<typename T>
T lua_to_c(lua_State* L, int i) {
    static_assert(std::is_pointer<T>::value, "type unknow");
    
    return (T)lua_touserdata(L, i);
}

template<>
bool lua_to_c<bool>(lua_State* L, int i) {
    return lua_toboolean(L, i) != 0;
}

template<>
char lua_to_c<char>(lua_State* L, int i) {
    return (char)lua_tointeger(L, i);
}

template<>
unsigned char lua_to_c<unsigned char>(lua_State* L, int i) {
    return (unsigned char)lua_tointeger(L, i);
}

template<>
short lua_to_c<short>(lua_State* L, int i) {
    return (short)lua_tointeger(L, i);
}

template<>
unsigned short lua_to_c<unsigned short>(lua_State* L, int i) {
    return (unsigned short)lua_tointeger(L, i);
}

template<>
int lua_to_c<int>(lua_State* L, int i) {
    return (int)lua_tointeger(L, i);
}

template<>
unsigned int lua_to_c<unsigned int>(lua_State* L, int i) {
    return (unsigned int)lua_tointeger(L, i);
}

template<>
long lua_to_c<long>(lua_State* L, int i) {
    return (long)lua_tointeger(L, i);
}

template<>
unsigned long lua_to_c<unsigned long>(lua_State* L, int i) {
    return (unsigned long)lua_tointeger(L, i);
}

template<>
long long lua_to_c<long long>(lua_State* L, int i) {
    return lua_tointeger(L, i);
}

template<>
unsigned long long
lua_to_c<unsigned long long>(lua_State* L, int i) {
    return (unsigned long long) lua_tointeger(L, i);
}

template<>
float lua_to_c<float>(lua_State* L, int i) {
    return (float)lua_tonumber(L, i);
}

template<>
double lua_to_c<double>(lua_State* L, int i) {
    return lua_tonumber(L, i);
}

template<>
const char* lua_to_c<const char*>(lua_State* L, int i) {
    return lua_tostring(L, i);
}

template<>
std::string lua_to_c<std::string>(lua_State* L, int i) {
    const char* str = lua_tostring(L, i);
    return str == nullptr ? "" : str;
}


template<typename T>
void c_to_lua(lua_State* L, T v) {
    static_assert(std::is_pointer<T>::value, "type unknow");
    lua_pushlightuserdata(L, v);
}

void c_to_lua(lua_State* L, bool v) {
    lua_pushboolean(L, v);
}

void c_to_lua(lua_State* L, char v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, unsigned char v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, short v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, unsigned short v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, int v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, unsigned int v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, long v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, unsigned long v) {
    lua_pushinteger(L, v);
}

void c_to_lua(lua_State* L, long long v) {
    lua_pushinteger(L, (lua_Integer)v);
}

void c_to_lua(lua_State* L, unsigned long long v) {
    lua_pushinteger(L, (lua_Integer)v);
}

void c_to_lua(lua_State* L, float v) {
    lua_pushnumber(L, v);
}

void c_to_lua(lua_State* L, double v) {
    lua_pushnumber(L, v);
}

void c_to_lua(lua_State* L, const char* v) {
    lua_pushstring(L, v);
}

void c_to_lua(lua_State* L, char* v) {
    lua_pushstring(L, v);
}

void c_to_lua(lua_State* L, const std::string& v) {
    lua_pushstring(L, v.c_str());
}


template <class T> class Register;
template<typename ReturnType, typename... Args>
class Register<ReturnType(*)(Args...)>
{
private:
    static constexpr auto indices = std::make_index_sequence<sizeof...(Args)>{};

    template <size_t... I, typename = std::enable_if_t<!std::is_void<ReturnType>::value>>
    static int caller(lua_State* L, ReturnType(*fp)(Args...), const std::index_sequence<I...>&) {
        c_to_lua(L, fp(lua_to_c<Args>(L, 1 + I)...));
        return 1;
    }
    template <size_t... I>
    static int caller(lua_State* L, void(*fp)(Args...), const std::index_sequence<I...>&) {
        fp(lua_to_c<Args>(L, 1 + I)...);
        return 0;
    }
public:
    template<auto fp>
    static int reg(lua_State* L) {
        return caller(L, fp, indices);
    }
};

template<auto fp, typename = std::enable_if_t<!std::is_same<decltype(fp), lua_CFunction>::value>>
void reg_global_func(lua_State* L, const char* name) {
    lua_register(L, name, Register<decltype(fp)>::template reg<fp>);
}

template<lua_CFunction fp>
void reg_global_func(lua_State* L, const char* name)
{
    lua_register(L, name, fp);
}

template <class T> class LClass
{
protected:
    /* 提供两种不同的注册函数,其返回值均为返回lua层的值数量 */
    typedef int32_t(T::* lua_CppFunction)(lua_State*);
public:
    virtual ~LClass() {}

    /* 在构造函数中不要调用virtual函数取c_new_func，只能当参数传进进来了 */
    explicit LClass(lua_State* L, const char* classname,
        lua_CFunction c_new_func = nullptr)
        : L(L)
    {
        _class_name = classname;
        /* lua 5.3的get函数基本返回类型，而5.1基本为void。需要另外调用is函数 */

        if (0 == luaL_newmetatable(L, _class_name))
        {
            FATAL("dumplicate define class %s\n", _class_name);
            return;
        }

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");

        lua_pushcfunction(L, tostring);
        lua_setfield(L, -2, "__tostring");

        /* metatable as value and pop metatable */
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        lua_newtable(L);
        lua_pushcfunction(L, c_new_func);
        lua_setfield(L, -2, "__call");
        lua_setmetatable(L, -2);

        lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
        if (!lua_istable(L, -1))
        {
            FATAL("define class before lua openlibs");
            return;
        }

        lua_pushvalue(L, 1);
        lua_setfield(L, -2, _class_name);

        lua_settop(L, 0);
    }

    /* 将c对象push栈,gc表示lua销毁userdata时，在gc函数中是否将当前指针delete
     * 由于此函数为static，但却依赖classname，而classname在构造函数中传入。
     * 因此，当调用类似lclass<lsocket>::push( L,_backend,false );的代码时，
     * 请保证你之前已注册对应的类，否则metatable将为nil
     */
    static int push(lua_State* L, const T* obj, bool gc = false)
    {
        assert(obj);
        assert(_class_name);

        /* 这里只是创建一个指针给lua管理，可以换用placement new把整个对象的
           内存都给lua管理
        */
        const T** ptr = (const T**)lua_newuserdata(L, sizeof(T*));
        *ptr = obj;

        C_LUA_OBJECT_ADD(_class_name);

        // 只有用lcalss定义了对应类的对象能push到lua，因此这里的metatable必须存在
        luaL_getmetatable(L, _class_name);

        /* 如果不自动gc，则需要在metatable中设置一张名为_notgc的表。以userdata
         * 为key的weaktable。当lua层调用gc时,userdata本身还存在，故这时判断是准确的
         */
        if (!gc)
        {
            subtable(L, 2, "_notgc", "k");

            lua_pushvalue(L, 1); /* 复制userdata到栈顶 */
            lua_pushboolean(L, 1);
            lua_settable(L, -3); /* _notgc[userdata] = true */

            lua_pop(L, 1); /* drop _notgc out of stack */
        }

        return lua_setmetatable(L, -2);
    }

    /* 注册函数,const char* func_name 就是注册到lua中的函数名字 */
    template <lua_CppFunction pf> LClass<T>& def(const char* func_name)
    {
        luaL_getmetatable(L, _class_name);

        lua_getfield(L, -1, func_name);
        if (!lua_isnil(L, -1))
        {
            ELOG("dumplicate def function %s:%s", _class_name, func_name);
        }
        lua_pop(L, 1); /* drop field */

        lua_pushcfunction(L, &fun_thunk<pf>);
        lua_setfield(L, -2, func_name);

        lua_pop(L, 1); /* drop class metatable */

        return *this;
    }

    /* 用于定义类的static函数 */
    template <lua_CFunction pf> LClass<T>& def(const char* func_name)
    {
        luaL_getmetatable(L, _class_name);

        lua_getfield(L, -1, func_name);
        if (!lua_isnil(L, -1))
        {
            ELOG("dumplicate def function %s:%s", _class_name, func_name);
        }
        lua_pop(L, 1); /* drop field */

        lua_pushcfunction(L, pf);
        lua_setfield(L, -2, func_name);

        lua_pop(L, 1); /* drop class metatable */

        return *this;
    }

    /* 注册变量,通常用于设置宏定义、枚举 */
    LClass<T>& set(int32_t val, const char* val_name)
    {
        luaL_getmetatable(L, _class_name);

        lua_getfield(L, -1, val_name);
        if (!lua_isnil(L, -1))
        {
            ELOG("dumplicate set variable %s:%s", _class_name, val_name);
        }
        lua_pop(L, 1); /* drop field */

        lua_pushinteger(L, val);
        lua_setfield(L, -2, val_name);

        lua_pop(L, 1); /* drop class metatable */

        return *this;
    }

private:
    /* 单例，不能创建c对象。可以直接在C中push对象到lua */
    static int32_t cnew(lua_State* L)
    {
        UNUSED(L);
        assert(false);
        return 0;
    }

    /* 元方法,__tostring */
    static int tostring(lua_State* L)
    {
        T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
        if (ptr != nullptr)
        {
            lua_pushfstring(L, "%s: %p", _class_name, *ptr);
            return 1;
        }
        return 0;
    }

    /* gc函数 */
    static int gc(lua_State* L)
    {
        C_LUA_OBJECT_DEC(_class_name);

        if (luaL_getmetafield(L, 1, "_notgc"))
        {
            /* 以userdata为key取值。如果未设置该userdata的_notgc值，则将会取得nil */
            lua_pushvalue(L, 1);
            lua_gettable(L, -2);
            /* gc = true表示执行gc函数 */
            if (lua_toboolean(L, -1)) return 0;
        }

        T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
        if (*ptr != nullptr) delete* ptr;
        *ptr = nullptr;

        return 0;
    }

    //创建弱表
    static void weaktable(lua_State* L, const char* mode)
    {
        lua_newtable(L);
        lua_pushvalue(L, -1); // table is its own metatable
        lua_setmetatable(L, -2);
        lua_pushliteral(L, "__mode");
        lua_pushstring(L, mode);
        lua_settable(L, -3); // metatable.__mode = mode
    }

    //创建子弱表
    static void subtable(lua_State* L, int tindex, const char* name,
        const char* mode)
    {
        lua_pushstring(L, name);
        lua_gettable(L, tindex); /* 判断是否已存在t[name] */

        if (lua_isnil(L, -1)) /* 不存在，则创建 */
        {
            lua_pop(L, 1); /* drop nil */
            weaktable(L, mode);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_settable(L, tindex); /* set t[name] */
        }
    }

    template <lua_CppFunction pf> static int fun_thunk(lua_State* L)
    {
        T** ptr = (T**)luaL_checkudata(
            L, 1, _class_name); /* get 'self', or if you prefer, 'this' */
        if (EXPECT_FALSE(ptr == nullptr || *ptr == nullptr))
        {
            return luaL_error(L, "%s calling method with null pointer",
                _class_name);
        }

        /* remove self so member function args start at index 1 */
        lua_remove(L, 1);

        return ((*ptr)->*pf)(L);
    }

protected:
    lua_State* L;
    static const char* _class_name;
};

// 构造函数指针不可获取，因为不知道是哪个重载
// 单例不允许创建
// 默认无参数构造函数
// 手动指定参数
/*
* lclass<Map> lc("Engine.Map"); // 默认无参数构造函数
* lclass<Map> lc<int, std::string>("Engine.Map"); // 指定构造参数
* lclass<Map> lc("Engine.map", nullptr); // 指定构造参数指针（nullptr为不允许构造）
*/