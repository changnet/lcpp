#pragma once

#include <lua.hpp>
#include <string>
#include <cassert>

namespace
{
    template<typename T>
    T lua_to_cpp(lua_State* L, int i)
    {
        static_assert(std::is_pointer<T>::value, "type unknow");

        // ����userdata����᷵��nullptr
        void* p = lua_touserdata(L, i);

        using T1 = typename std::remove_pointer_t<T>;
        if constexpr (std::is_void_v<T1>)
        {
            return p;
        }
        else
        {
            if (!p || lua_islightuserdata(L, i)) return (T)p;

            // ����ֻ����full userdata�ˣ�������������ͨ��lclass push��ָ��
            const char* name = LClass<T1>::template _class_name;
            if (luaL_testudata(L, i, name)) return *((T1**)p);

            return nullptr;
        }
    }

    template<>
    bool lua_to_cpp<bool>(lua_State* L, int i)
    {
        return lua_toboolean(L, i) != 0;
    }

    template<>
    char lua_to_cpp<char>(lua_State* L, int i)
    {
        return (char)lua_tointeger(L, i);
    }

    template<>
    unsigned char lua_to_cpp<unsigned char>(lua_State* L, int i)
    {
        return (unsigned char)lua_tointeger(L, i);
    }

    template<>
    short lua_to_cpp<short>(lua_State* L, int i)
    {
        return (short)lua_tointeger(L, i);
    }

    template<>
    unsigned short lua_to_cpp<unsigned short>(lua_State* L, int i)
    {
        return (unsigned short)lua_tointeger(L, i);
    }

    template<>
    int lua_to_cpp<int>(lua_State* L, int i)
    {
        return (int)lua_tointeger(L, i);
    }

    template<>
    unsigned int lua_to_cpp<unsigned int>(lua_State* L, int i)
    {
        return (unsigned int)lua_tointeger(L, i);
    }

    template<>
    long lua_to_cpp<long>(lua_State* L, int i)
    {
        return (long)lua_tointeger(L, i);
    }

    template<>
    unsigned long lua_to_cpp<unsigned long>(lua_State* L, int i)
    {
        return (unsigned long)lua_tointeger(L, i);
    }

    template<>
    long long lua_to_cpp<long long>(lua_State* L, int i)
    {
        return lua_tointeger(L, i);
    }

    template<>
    unsigned long long
        lua_to_cpp<unsigned long long>(lua_State* L, int i)
    {
        return (unsigned long long) lua_tointeger(L, i);
    }

    template<>
    float lua_to_cpp<float>(lua_State* L, int i)
    {
        return (float)lua_tonumber(L, i);
    }

    template<>
    double lua_to_cpp<double>(lua_State* L, int i)
    {
        return lua_tonumber(L, i);
    }

    template<>
    const char* lua_to_cpp<const char*>(lua_State* L, int i)
    {
        // ����Ҫע�⣬�޷�ת��Ϊconst char*�᷵��NULL
        // ������Ͷ�c++������ȫ������ std::cout << NULL�ͻ�ֱ�ӵ�������ʱ��Ҫ���ָ��
        return lua_tostring(L, i);
    }

    template<>
    char* lua_to_cpp<char*>(lua_State* L, int i)
    {
        // ����Ҫע�⣬�޷�ת��Ϊconst char*�᷵��NULL
        // ������Ͷ�c++������ȫ������ std::cout << NULL�ͻ�ֱ�ӵ�������ʱ��Ҫ���ָ��
        return const_cast<char *>(lua_tostring(L, i));
    }

    template<>
    std::string lua_to_cpp<std::string>(lua_State* L, int i)
    {
        const char* str = lua_tostring(L, i);
        return str == nullptr ? "" : str;
    }

    template<typename T>
    void cpp_to_lua(lua_State* L, T v)
    {
        static_assert(std::is_pointer<T>::value, "type unknow");

        // �������������࣬����push��ʽ���͵�lua
        using T1 = typename std::remove_pointer_t<T>;
        if constexpr (std::is_void_v<T1>)
        {
            lua_pushlightuserdata(L, v);
        }
        else
        {
            const char* name = LClass<T1>::template _class_name;
            if (name)
            {
                LClass<T1>::push(L, v);
            }
            else
            {
                lua_pushlightuserdata(L, v);
            }
        }
    }

    void cpp_to_lua(lua_State* L, bool v)
    {
        lua_pushboolean(L, v);
    }

    void cpp_to_lua(lua_State* L, char v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, unsigned char v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, short v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, unsigned short v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, int v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, unsigned int v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, long v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, unsigned long v)
    {
        lua_pushinteger(L, v);
    }

    void cpp_to_lua(lua_State* L, long long v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    void cpp_to_lua(lua_State* L, unsigned long long v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    void cpp_to_lua(lua_State* L, float v)
    {
        lua_pushnumber(L, v);
    }

    void cpp_to_lua(lua_State* L, double v)
    {
        lua_pushnumber(L, v);
    }

    void cpp_to_lua(lua_State* L, const char* v)
    {
        lua_pushstring(L, v);
    }

    void cpp_to_lua(lua_State* L, char* v)
    {
        lua_pushstring(L, v);
    }

    void cpp_to_lua(lua_State* L, const std::string& v)
    {
        lua_pushstring(L, v.c_str());
    }

    // ǰ������
    template <typename T> struct class_remove;
    // �ػ�Ϊstatic������ȫ�ֺ���
    /*template<typename Ret, typename... Args>
    struct class_remove<Ret(*)(Args...)>
    {
        using type = Ret(*)(Args...);
    };
    */
    // �ػ�Ϊ��Ա����
    template<typename T, typename Ret, typename... Args>
    struct class_remove<Ret(T::*)(Args...)>
    {
        using type = Ret(*)(Args...);
    };
    // �ػ�Ϊconst��Ա����
    template<typename T, typename Ret, typename... Args>
    struct class_remove<Ret(T::*)(Args...) const> : public class_remove<Ret(T::*)(Args...)>
    {};

    template<typename T>
    inline constexpr bool is_lua_func = std::is_same<typename class_remove<T>::type, lua_CFunction>::value;

    // C++ 20 std::remove_cvref
    // TODO std::remove_volatile ��Ҫ��
    template <typename T>
    using remove_cvref = std::remove_cv<std::remove_reference_t<T>>::template type;
    template <class T> class Register;
    template<typename Ret, typename... Args>
    class Register<Ret(*)(Args...)>
    {
    private:
        static constexpr auto indices = std::make_index_sequence<sizeof...(Args)>{};

        template <size_t... I, typename = std::enable_if_t<!std::is_void<Ret>::value>>
        static int caller(lua_State* L, Ret(*fp)(Args...), const std::index_sequence<I...>&)
        {
            cpp_to_lua(L, fp(lua_to_cpp<remove_cvref<Args>>(L, 1 + I)...));
            return 1;
        }
        template <size_t... I>
        static int caller(lua_State* L, void(*fp)(Args...), const std::index_sequence<I...>&)
        {
            fp(lua_to_cpp<remove_cvref<Args>>(L, 1 + I)...);
            return 0;
        }
    public:
        template<auto fp>
        static int reg(lua_State* L)
        {
            return caller(L, fp, indices);
        }
    };
}

namespace lclass
{
    template<auto fp>
    void reg_global_func(lua_State* L, const char* name)
    {
        if constexpr (std::is_same_v<decltype(fp), lua_CFunction>)
        {
            lua_register(L, name, fp);
            
        }
        else
        {
            lua_register(L, name, Register<decltype(fp)>::template reg<fp>);
        }
    }
}

template <class T>
class LClass
{
private:
    using lua_CppFunction = int32_t(T::*)(lua_State*);

    template <class U> class StaticRegister;
    template<typename Ret, typename... Args>
    class StaticRegister<Ret(*)(Args...)>
    {
    private:
        static constexpr auto indices = std::make_index_sequence<sizeof...(Args)>{};

        template <auto fp, size_t... I>
        static int caller(lua_State* L, const std::index_sequence<I...>&)
        {
            if constexpr (std::is_void_v<Ret>)
            {
                fp(lua_to_cpp<remove_cvref<Args>>(L, 2 + I)...);
                return 0;
            }
            else
            {
                cpp_to_lua(L, fp(lua_to_cpp<remove_cvref<Args>>(L, 2 + I)...));
                return 1;
            }
        }

    public:
        template<auto fp>
        static int reg(lua_State* L)
        {
            return caller<fp>(L, indices);
        }
    };

    template <typename FT> class ClassRegister;

    template<typename FT, typename Ret, typename... Args>
    class ClassRegister<Ret(FT::*)(Args...)>
    {
    private:
        static constexpr auto indices = std::make_index_sequence<sizeof...(Args)>{};

        template <auto fp, size_t... I>
        static int caller(lua_State* L, const std::index_sequence<I...>&)
        {
            T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
            if (ptr == nullptr || *ptr == nullptr)
            {
                return luaL_error(L, "%s calling method with null pointer", _class_name);
            }

            // ʹ��if constexpr�滻���ģ���ά��һЩ
            // template <size_t... I, typename = std::enable_if_t<!std::is_void<Ret>::value>>

            if constexpr (std::is_void_v<Ret>)
            {
                ((*ptr)->*fp)(lua_to_cpp<remove_cvref<Args>>(L, 2 + I)...);
                return 0;
            }
            else
            {
                cpp_to_lua(L, ((*ptr)->*fp)(lua_to_cpp<remove_cvref<Args>>(L, 2 + I)...));
                return 1;
            }
        }

    public:
        template<auto fp>
        static int reg(lua_State* L)
        {
            return caller<fp>(L, indices);
        }
    };

    template<typename FT, typename Ret, typename... Args>
    class ClassRegister<Ret(FT::*)(Args...) const> : public ClassRegister<Ret(FT::*)(Args...)>
    {};

public:
    virtual ~LClass() {}

    // ����һ����Ķ��󣬵�����luaע�ᡣ������ע���ʹ��ͬ���Ķ����������LӦ�ú�ע��ʱһ��
    explicit LClass(lua_State* L) : L(L)
    {
        // ע����󣬱ض���������
        // assert��_class_name);
    }

    // ע��һ����
    // @param L lua�����ָ��
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
        ʹ��Lua����һ���࣬��Ҫ�õ�LuaԪ���е�__index���ơ�__index��Ԫ���һ��key��value
        Ϊһ��table�����漴�����ĸ�����������˵����������һ����ĺ���ʱ����Ҫ�жϻ�ȡmetatable
        ���ٻ�ȡ__index���table�����ղ�ȡ�ö�Ӧ�ĺ���ȥ���á�

        ������Ƶ�����������ʵ�ǱȽϴ��

        local tbl = {
            v1, v2, -- tbl��������ݷ�tbl��

            -- Ԫ������
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

        // ����һ��table��Ϊmetatable��Ԫ������ metatable() ���ܴ���һ������
        // ����д����C++һ��
        // if constexpr �Ǳ���ʱ���ɣ�����û��Ĭ�Ϲ��캯�����������κβ��������࣬�ǲ���ע��__call��
        if constexpr (std::is_constructible_v<T>)
        {
            lua_newtable(L);
            lua_pushcfunction(L, class_constructor<>);
            lua_setfield(L, -2, "__call");
            lua_setmetatable(L, -2);
        }

        /*
        __index����Ҫ����һ��table�����溯������Ϊ�˽�ʡ�ڴ棬�� metatable.__index = metatable��
        ����func1��func2��__gc�Ⱥ�������һ�𣬲���Ҫ���ⴴ��һ��table��

        ����ע�⣬�������Ϊ�˸���__gc��__tostring�����ú������벻Ҫ���������֡�����һ��featureҲ��һ����
         */
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        // ����loaded��������Lua�п�������ͨģ������require "xxx"
        lua_setfield(L, -2, _class_name);

        lua_pop(L, 1); // drop _loaded table
    }

    // ָ�����캯���Ĳ���
    template<typename... Args>
    void constructor()
    {
        luaL_getmetatable(L, _class_name);
        assert(lua_istable(L, -1));

        lua_getmetatable(L, -1);
        if (!lua_istable(L, -1))
        {
            lua_newtable(L);
        }
        lua_pushcfunction(L, class_constructor<Args...>);
        lua_setfield(L, -2, "__call");
        lua_setmetatable(L, -2);

        lua_pop(L, 1); /* drop class metatable */
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

        /* ������Զ�gc������Ҫ��metatable������һ����Ϊ_notgc�ı���userdata
         * Ϊkey��weaktable����lua�����gcʱ,userdata�������ڣ�����ʱ�ж���׼ȷ��
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

    // ��һ������ָ��push��ȫ�ֱ���
    static int push_global(lua_State* L, const T* obj, const char* name, bool gc = false)
    {
        if (0 != push(L, obj, gc)) return -1;

        lua_setglobal(L, name);
        return 0;
    }

    template<auto fp>
    void def(const char* name)
    {
        lua_CFunction cfp = nullptr;
        if constexpr (std::is_same_v<decltype(fp), lua_CFunction>)
        {
            cfp = fp;
        }
        else if constexpr (!std::is_member_function_pointer_v<decltype(fp)>)
        {
            cfp = StaticRegister<decltype(fp)>::template reg<fp>;
        }
        else if constexpr (is_lua_func<decltype(fp)>)
        {
            cfp = &fun_thunk<fp>;
        }
        else
        {
            cfp = ClassRegister<decltype(fp)>::template reg<fp>;
        }

        luaL_getmetatable(L, _class_name);

        lua_pushcfunction(L, cfp);
        lua_setfield(L, -2, name);

        lua_pop(L, 1); /* drop class metatable */
    }

    /* ע�����,ͨ���������ú궨�塢ö�� */
    void set(int32_t val, const char* val_name)
    {
        luaL_getmetatable(L, _class_name);

        lua_pushinteger(L, val);
        lua_setfield(L, -2, val_name);

        lua_pop(L, 1); /* drop class metatable */
    }

private:
    template <typename... Args, size_t... I>
    static T * class_constructor_caller(lua_State* L, const std::index_sequence<I...>&)
    {
        return new T(lua_to_cpp<Args>(L, 2 + I)...);
    }

    template<typename... Args>
    static int class_constructor(lua_State* L)
    {
        T* obj = class_constructor_caller<Args...>(
            L, std::make_index_sequence<sizeof...(Args)>{});

        // lua����__call,��һ��������Ԫ��
        // ������й��캯������,ֻ����Ԫ��(TODO: �Ƿ�Ҫ���)
        lua_settop(L, 1); /*  */

        T** ptr = (T**)lua_newuserdata(L, sizeof(T*));
        *ptr = obj;

        /* ���´�����userdata��Ԫ������ջλ�� */
        lua_insert(L, 1);

        /* ��Ԫ������Ϊuserdata��Ԫ��������Ԫ�� */
        lua_setmetatable(L, -2);

        return 1;
    }

    // ��һ������ת��Ϊһ��light userdata
    static int toludata(lua_State* L)
    {
        T** ptr = (T**)luaL_checkudata(L, 1, _class_name);

        lua_pushlightuserdata(L, *ptr);
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
    static void subtable(lua_State* L, int index, const char* name,
        const char* mode)
    {
        lua_pushstring(L, name);
        lua_rawget(L, index); /* �ж��Ƿ��Ѵ���t[name] */

        if (lua_isnil(L, -1)) /* �����ڣ��򴴽� */
        {
            lua_pop(L, 1); /* drop nil */
            weaktable(L, mode);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_rawset(L, index); /* set t[name] */
        }
    }

    template <auto pf>
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

public:
    static const char* _class_name;
private:
    lua_State* L;
};
template <class T> const char* LClass<T>::_class_name = nullptr;
