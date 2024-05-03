#pragma once

#include <lua.hpp>
#include <string>
#include <cassert>

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

template<auto fp, typename = std::enable_if_t<!std::is_same_v<decltype(fp), lua_CFunction>>>
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
private:
    using lua_CppFunction = int32_t(T::*)(lua_State*);
public:
    virtual ~LClass() {}

    /*
     * 注册一个类型，使用默认无参数构造函数创建对象 
     */
    explicit LClass(lua_State* L, const char* classname)
        : L(L)
    {
        _class_name = classname;

        lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
        assert(lua_istable(L, -1));

        // 该类名已经注册过
        if (0 == luaL_newmetatable(L, _class_name))
        {
            assert(false);
            return;
        }

        /*
        使用Lua创建一个类，需要用到Lua元表中的__index机制。__index是元表的一个key，value
        为一个table，里面即这个类的各个函数。这说明，当访问一个类的函数时，需要判断获取metatable
        ，再获取__index这个table，最终才取得对应的函数去调用。

        这个机制的性能消耗其实是比较大的

        local tbl = {
            v1, v2, -- tbl对象的数据放tbl里

            -- 元表负责函数
            metatable = {
                __gc = xx,
                __tostring = xx,
                __index = {
                    func1 = xx,
                    func2 = xx,
                },
                metatable = {
                    __call = xx,
                }
            }
        }
         */

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");

        lua_pushcfunction(L, tostring);
        lua_setfield(L, -2, "__tostring");

        lua_pushcfunction(L, toludata);
        lua_setfield(L, -2, "toludata");

        // 创建一个table作为metatable的元表，这样 metatable() 就能创建一个对象
        // 写法和C++一样
        lua_newtable(L);
        lua_pushcfunction(L, new_class_obj);
        lua_setfield(L, -2, "__call");
        lua_setmetatable(L, -2);

        /*
        __index还需要创建一个table来保存函数，但为了节省内存，让 metatable.__index = metatable，
        这样func1和func2和__gc等函数放在一起，不需要额外创建一个table。

        但请注意，如果不是为了覆盖__gc、__tostring等内置函数，请不要用这种名字。这是一个feature也是一个坑
         */
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        // 设置loaded，这样在Lua中可以像普通模块那样require "xxx"
        lua_setfield(L, -2, _class_name);

        lua_pop(L, 1); // drop _loaded table
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

        /* 这里只是创建一个指针给lua管理
        */
        const T** ptr = (const T**)lua_newuserdata(L, sizeof(T*));
        *ptr = obj;

        // 只有用lcalss定义了对应类的对象能push到lua，因此这里的metatable必须存在
        luaL_getmetatable(L, _class_name);
        if (!lua_istable(L, -1))
        {
            return -1;
        }

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

        lua_setmetatable(L, -2);
        return 0;
    }

    /*
     * 定义格式为 int (T::*)(lua_State *L) 的成员函数，注意取参数时，需要从第2个开始取
     */
    template <lua_CppFunction pf>
    void def(const char* name)
    {
        luaL_getmetatable(L, _class_name);

        lua_pushcfunction(L, &fun_thunk<pf>);
        lua_setfield(L, -2, name);

        lua_pop(L, 1); /* drop class metatable */
    }

    /* 定义类的static函数 */
    template <lua_CFunction pf>
    void def(const char* name)
    {
        luaL_getmetatable(L, _class_name);

        lua_pushcfunction(L, pf);
        lua_setfield(L, -2, name);

        lua_pop(L, 1); /* drop class metatable */
    }

    /* 注册变量,通常用于设置宏定义、枚举 */
    void set(int32_t val, const char* val_name)
    {
        luaL_getmetatable(L, _class_name);

        lua_pushinteger(L, val);
        lua_setfield(L, -2, val_name);

        lua_pop(L, 1); /* drop class metatable */
    }

private:
    /* 创建c对象 */
    static int new_class_obj(lua_State* L)
    {
        /* lua调用__call,第一个参数是元表 */
        T* obj = new T();

        lua_settop(L, 1); /* 清除所有构造函数参数,只保留元表 */

        T** ptr = (T**)lua_newuserdata(L, sizeof(T*));
        *ptr = obj;

        /* 把新创建的userdata和元表交换堆栈位置 */
        lua_insert(L, 1);

        /* 把元表设置为userdata的元表，并弹出元表 */
        lua_setmetatable(L, -2);

        return 1;
    }

    // 把一个对象转换为一个light userdata
    static int toludata(lua_State* L)
    {
        T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
        
        lua_pushlightuserdata(L, *ptr);
        return 1;
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
    static void subtable(lua_State* L, int index, const char* name,
        const char* mode)
    {
        lua_pushstring(L, name);
        lua_rawget(L, index); /* 判断是否已存在t[name] */

        if (lua_isnil(L, -1)) /* 不存在，则创建 */
        {
            lua_pop(L, 1); /* drop nil */
            weaktable(L, mode);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_rawset(L, index); /* set t[name] */
        }
    }

    template <lua_CppFunction pf>
    static int fun_thunk(lua_State* L)
    {
        T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
        if (ptr == nullptr || *ptr == nullptr)
        {
            return luaL_error(L, "%s calling method with null pointer",
                _class_name);
        }

        return ((*ptr)->*pf)(L);
    }

private:
    lua_State* L;
    static const char* _class_name;
};
template <class T> const char* LClass<T>::_class_name = nullptr;

// 构造函数指针不可获取，因为不知道是哪个重载
// 单例不允许创建
// 默认无参数构造函数
// 手动指定参数
/*
* lclass<Map> lc("Engine.Map"); // 默认无参数构造函数
* lclass<Map> lc<int, std::string>("Engine.Map"); // 指定构造参数
* lclass<Map> lc("Engine.map", nullptr); // 指定构造参数指针（nullptr为不允许构造）
*/