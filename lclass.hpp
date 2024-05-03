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
    /* �ṩ���ֲ�ͬ��ע�ắ��,�䷵��ֵ��Ϊ����lua���ֵ���� */
    typedef int32_t(T::* lua_CppFunction)(lua_State*);
public:
    virtual ~LClass() {}

    /*
     * ע��һ�����ͣ�ʹ��Ĭ���޲������캯���������� 
     */
    explicit LClass(lua_State* L, const char* classname)
        : L(L)
    {
        _class_name = classname;

        lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
        assert(lua_istable(L, -1));

        // �������Ѿ�ע���
        if (0 == luaL_newmetatable(L, _class_name))
        {
            assert(false);
            return;
        }

        /*
        ʹ��Lua����һ���࣬��Ҫ�õ�LuaԪ���е�__index���ơ�__index��Ԫ����һ��key��value
        Ϊһ��table�����漴�����ĸ�����������˵����������һ����ĺ���ʱ����Ҫ�жϻ�ȡmetatable
        ���ٻ�ȡ__index���table�����ղ�ȡ�ö�Ӧ�ĺ���ȥ���á�

        ������Ƶ�����������ʵ�ǱȽϴ��

        local tbl = {
            v1, v2, -- tbl��������ݷ�tbl��

            -- Ԫ��������
            metatable = {
                __gc = xx,
                __tostring = xx,
                __index = {
                    func1 = xx,
                    func2 = xx,
                },
            }
        }
         */

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");

        lua_pushcfunction(L, tostring);
        lua_setfield(L, -2, "__tostring");

        lua_pushcfunction(L, c_new);
        lua_setfield(L, -2, "__call");

        /*
        __index����Ҫ����һ��table�����溯������Ϊ�˽�ʡ�ڴ棬�� metatable.__index = metatable��
        ����func1��func2��__gc�Ⱥ�������һ�𣬲���Ҫ���ⴴ��һ��table��

        ����ע�⣬�������Ϊ�˸���__gc��__tostring�����ú������벻Ҫ���������֡�����һ��featureҲ��һ����
         */
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        // ����loaded��������Lua�п�������ͨģ������require "xxx"
        lua_setfield(L, -2, _class_name);
    }

    /* ��c����pushջ,gc��ʾlua����userdataʱ����gc�������Ƿ񽫵�ǰָ��delete
     * ���ڴ˺���Ϊstatic����ȴ����classname����classname�ڹ��캯���д��롣
     * ��ˣ�����������lclass<lsocket>::push( L,_backend,false );�Ĵ���ʱ��
     * �뱣֤��֮ǰ��ע���Ӧ���࣬����metatable��Ϊnil
     */
    static int push(lua_State* L, const T* obj, bool gc = false)
    {
        assert(obj);
        assert(_class_name);

        /* ����ֻ�Ǵ���һ��ָ���lua����
        */
        const T** ptr = (const T**)lua_newuserdata(L, sizeof(T*));
        *ptr = obj;

        // ֻ����lcalss�����˶�Ӧ��Ķ�����push��lua����������metatable�������
        luaL_getmetatable(L, _class_name);
        if (!lua_istable(L, -1))
        {
            return -1;
        }

        /* ������Զ�gc������Ҫ��metatable������һ����Ϊ_notgc�ı�����userdata
         * Ϊkey��weaktable����lua�����gcʱ,userdata���������ڣ�����ʱ�ж���׼ȷ��
         */
        if (!gc)
        {
            subtable(L, 2, "_notgc", "k");

            lua_pushvalue(L, 1); /* ����userdata��ջ�� */
            lua_pushboolean(L, 1);
            lua_settable(L, -3); /* _notgc[userdata] = true */

            lua_pop(L, 1); /* drop _notgc out of stack */
        }

        lua_setmetatable(L, -2);
        return 0;
    }

    /* ע�ắ��,const char* func_name ����ע�ᵽlua�еĺ������� */
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

    /* ���ڶ������static���� */
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

    /* ע�����,ͨ���������ú궨�塢ö�� */
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
    /* ����c���� */
    static int c_new(lua_State* L)
    {
        /* ���ȼ������ڹ��캯������luaL_errorִ��longjump�����ڴ�й©
         * ����������ͳ�Ƶ�
         */
        C_LUA_OBJECT_ADD(_class_name);

        /* lua����__call,��һ�������Ǹ�Ԫ��������table.ȡ���캯������Ҫע�� */
        T* obj = new T();

        lua_settop(L, 1); /* ������й��캯������,ֻ����Ԫ�� */

        T** ptr = (T**)lua_newuserdata(L, sizeof(T*));
        *ptr = obj;

        /* ���´�����userdata��Ԫ��������ջλ�� */
        lua_insert(L, 1);

        /* ����Ԫ��,����Ԫ������Ϊuserdata��Ԫ�� */
        lua_setmetatable(L, -2);

        return 1;
    }

    /* Ԫ����,__tostring */
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

    /* gc���� */
    static int gc(lua_State* L)
    {
        C_LUA_OBJECT_DEC(_class_name);

        if (luaL_getmetafield(L, 1, "_notgc"))
        {
            /* ��userdataΪkeyȡֵ�����δ���ø�userdata��_notgcֵ���򽫻�ȡ��nil */
            lua_pushvalue(L, 1);
            lua_gettable(L, -2);
            /* gc = true��ʾִ��gc���� */
            if (lua_toboolean(L, -1)) return 0;
        }

        T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
        if (*ptr != nullptr) delete* ptr;
        *ptr = nullptr;

        return 0;
    }

    //��������
    static void weaktable(lua_State* L, const char* mode)
    {
        lua_newtable(L);
        lua_pushvalue(L, -1); // table is its own metatable
        lua_setmetatable(L, -2);
        lua_pushliteral(L, "__mode");
        lua_pushstring(L, mode);
        lua_settable(L, -3); // metatable.__mode = mode
    }

    //����������
    static void subtable(lua_State* L, int tindex, const char* name,
        const char* mode)
    {
        lua_pushstring(L, name);
        lua_gettable(L, tindex); /* �ж��Ƿ��Ѵ���t[name] */

        if (lua_isnil(L, -1)) /* �����ڣ��򴴽� */
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

// ���캯��ָ�벻�ɻ�ȡ����Ϊ��֪�����ĸ�����
// ��������������
// Ĭ���޲������캯��
// �ֶ�ָ������
/*
* lclass<Map> lc("Engine.Map"); // Ĭ���޲������캯��
* lclass<Map> lc<int, std::string>("Engine.Map"); // ָ���������
* lclass<Map> lc("Engine.map", nullptr); // ָ���������ָ�루nullptrΪ���������죩
*/