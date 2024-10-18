#pragma once

#include <lua.hpp>
#include <string>
#include <cassert>
#include <stdexcept>

#define ARGS_CHECK

namespace lcpp
{

    // �����ļ�⣬����ֱ����lua_check*����Ϊ�ᴥ��long jump��������throw
#ifdef ARGS_CHECK

    static inline void throw_error(lua_State* L, const char* name, int i)
    {
        char buff[256];
        snprintf(buff, sizeof(buff), "bad argument #%d, %s expected, got %s",
                i, name, lua_typename(L, lua_type(L,i)));
        throw std::runtime_error(buff);
    }
#define luaL_checkis(t)                                              \
        if (!lua_is##t(L, i)) throw_error(L, #t, i); else (void)0
#else
#define luaL_checkis(t) (void)0
#endif

    template <typename T> T lua_to_cpp(lua_State* L, int i)
    {
        static_assert(std::is_pointer<T>::value, "type unknow");

        luaL_checkis(userdata);

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
            const char* name = Class<T1>::template _class_name;
            if (luaL_testudata(L, i, name)) return *((T1**)p);

            return nullptr;
        }
    }

    template <> inline bool lua_to_cpp<bool>(lua_State* L, int i)
    {
        luaL_checkis(boolean);
        return lua_toboolean(L, i) != 0;
    }

    template <> inline char lua_to_cpp<char>(lua_State* L, int i)
    {
        luaL_checkis(string);
        const char* str = lua_tostring(L, i);
        return *str;
    }

    template <> inline unsigned char lua_to_cpp<unsigned char>(lua_State* L, int i)
    {
        luaL_checkis(string);
        const char* str = lua_tostring(L, i);
        return (unsigned char)*str;
    }

    template <> inline short lua_to_cpp<short>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (short)lua_tointeger(L, i);
    }

    template <>
    inline unsigned short lua_to_cpp<unsigned short>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (unsigned short)lua_tointeger(L, i);
    }

    template <> inline int lua_to_cpp<int>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (int)lua_tointeger(L, i);
    }

    template <> inline unsigned int lua_to_cpp<unsigned int>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (unsigned int)lua_tointeger(L, i);
    }

    template <> inline long lua_to_cpp<long>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (long)lua_tointeger(L, i);
    }

    template <> inline unsigned long lua_to_cpp<unsigned long>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (unsigned long)lua_tointeger(L, i);
    }

    template <> inline long long lua_to_cpp<long long>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return lua_tointeger(L, i);
    }

    template <>
    inline unsigned long long lua_to_cpp<unsigned long long>(lua_State* L, int i)
    {
        luaL_checkis(number); // lua���������1.0���ж�Ϊnumber�����������ֻҪ�����־�����
        return (unsigned long long)lua_tointeger(L, i);
    }

    template <> inline float lua_to_cpp<float>(lua_State* L, int i)
    {
        luaL_checkis(number);
        return (float)lua_tonumber(L, i);
    }

    template <> inline double lua_to_cpp<double>(lua_State* L, int i)
    {
        luaL_checkis(number);
        return lua_tonumber(L, i);
    }

    template <> inline const char* lua_to_cpp<const char*>(lua_State* L, int i)
    {
        // ����Ҫע�⣬�޷�ת��Ϊconst char*�᷵��NULL
        // ������Ͷ�c++������ȫ������ std::cout << NULL�ͻ�ֱ�ӵ�������ʱ��Ҫ���ָ��
        return lua_tostring(L, i);
    }

    template <> inline char* lua_to_cpp<char*>(lua_State* L, int i)
    {
        // ����Ҫע�⣬�޷�ת��Ϊconst char*�᷵��NULL
        // ������Ͷ�c++������ȫ������ std::cout << NULL�ͻ�ֱ�ӵ�������ʱ��Ҫ���ָ��
        luaL_checkis(string);
        return const_cast<char*>(lua_tostring(L, i));
    }

    template <> inline std::string lua_to_cpp<std::string>(lua_State* L, int i)
    {
        luaL_checkis(string);
        const char* str = lua_tostring(L, i);
        return str == nullptr ? "" : str;
    }

    template <typename T> void cpp_to_lua(lua_State* L, T v)
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
            const char* name = Class<T1>::template _class_name;
            if (name)
            {
                Class<T1>::push(L, v);
            }
            else
            {
                lua_pushlightuserdata(L, v);
            }
        }
    }

    inline void cpp_to_lua(lua_State* L, bool v)
    {
        lua_pushboolean(L, v);
    }

    inline void cpp_to_lua(lua_State* L, char v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, unsigned char v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, short v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, unsigned short v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, int v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, unsigned int v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, long v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, unsigned long v)
    {
        lua_pushinteger(L, v);
    }

    inline void cpp_to_lua(lua_State* L, long long v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    inline void cpp_to_lua(lua_State* L, unsigned long long v)
    {
        lua_pushinteger(L, (lua_Integer)v);
    }

    inline void cpp_to_lua(lua_State* L, float v)
    {
        lua_pushnumber(L, v);
    }

    inline void cpp_to_lua(lua_State* L, double v)
    {
        lua_pushnumber(L, v);
    }

    inline void cpp_to_lua(lua_State* L, const char* v)
    {
        lua_pushstring(L, v);
    }

    inline void cpp_to_lua(lua_State* L, char* v)
    {
        lua_pushstring(L, v);
    }

    inline void cpp_to_lua(lua_State* L, const std::string& v)
    {
        lua_pushstring(L, v.c_str());
    }

    // C++ 20 std::remove_cvref
    // TODO std::remove_volatile ��Ҫ��
    template <typename T>
    using remove_cvref = std::remove_cv<std::remove_reference_t<T>>::template type;

    /**
     * @brief ����ȫ�ֺ�����static����ע��
     */
    template <class T> class Register;
    template <typename Ret, typename... Args> class Register<Ret(*)(Args...)>
    {
    private:
        static constexpr auto indices = std::make_index_sequence<sizeof...(Args)>{};

        template <size_t... I, typename = std::enable_if_t<!std::is_void<Ret>::value>>
        static int caller(lua_State* L, Ret(*fp)(Args...),
            const std::index_sequence<I...>&)
        {
            cpp_to_lua(L, fp(lua_to_cpp<remove_cvref<Args>>(L, 1 + I)...));
            return 1;
        }
        template <size_t... I>
        static int caller(lua_State* L, void (*fp)(Args...),
            const std::index_sequence<I...>&)
        {
            fp(lua_to_cpp<remove_cvref<Args>>(L, 1 + I)...);
            return 0;
        }

    public:
        template <auto fp> static int reg(lua_State* L)
        {
            try
            {
                return caller(L, fp, indices);
            }
            catch (const std::runtime_error& e)
            {
                // ����������ε������е�C++����Ӧ�ö����ͷţ����԰�ȫlong jump��
                return luaL_error(L, e.what());
            }
        }
    };

    /**
     * @brief ����C++��ע��
     */
    template <class T> class Class
    {
    private:
        // ����C++���е�static����ע��
        template <class C> class StaticRegister;
        template <typename Ret, typename... Args>
        class StaticRegister<Ret(*)(Args...)>
        {
        private:
            static constexpr auto indices =
                std::make_index_sequence<sizeof...(Args)>{};

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
            template <auto fp> static int reg(lua_State* L)
            {
                try
                {
                    return caller<fp>(L, indices);
                }
                catch (const std::runtime_error& e)
                {
                    // ����������ε������е�C++����Ӧ�ö����ͷţ����԰�ȫlong jump��
                    return luaL_error(L, e.what());
                }
            }
        };

        // ��ͨC++��ע��
        template <typename C> class ClassRegister;
        template <typename C, typename Ret, typename... Args>
        class ClassRegister<Ret(C::*)(Args...)>
        {
        private:
            static constexpr auto indices =
                std::make_index_sequence<sizeof...(Args)>{};

            template <auto fp, size_t... I>
            static int caller(lua_State* L, const std::index_sequence<I...>&)
            {
                T** ptr = (T**)luaL_checkudata(L, 1, _class_name);
                if (ptr == nullptr || *ptr == nullptr)
                {
                    return luaL_error(L, "%s calling method with null pointer",
                        _class_name);
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
                    cpp_to_lua(L, ((*ptr)->*fp)(
                        lua_to_cpp<remove_cvref<Args>>(L, 2 + I)...));
                    return 1;
                }
            }

        public:
            template <auto fp> static int reg(lua_State* L)
            {
                try
                {
                    return caller<fp>(L, indices);
                }
                catch (const std::runtime_error& e)
                {
                    // ����������ε������е�C++����Ӧ�ö����ͷţ����԰�ȫlong jump��
                    return luaL_error(L, e.what());
                }
            }
        };

        template <typename C, typename Ret, typename... Args>
        class ClassRegister<Ret(C::*)(Args...) const>
            : public ClassRegister<Ret(C::*)(Args...)>
        {
        };

    public:
        virtual ~Class()
        {
        }

        // ����һ����Ķ��󣬵�����luaע�ᡣ������ע���ʹ��ͬ���Ķ����������LӦ�ú�ע��ʱһ��
        explicit Class(lua_State* L) : L(L)
        {
            // ע����󣬱ض���������
            // assert��_class_name);
        }

        // ע��һ����
        // @param L lua�����ָ��
        explicit Class(lua_State* L, const char* classname) : L(L)
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
            __index����Ҫ����һ��table�����溯������Ϊ�˽�ʡ�ڴ棬��
            metatable.__index = metatable��
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
        template <typename... Args> void constructor()
        {

            luaL_getmetatable(L, _class_name);
            assert(lua_istable(L, -1));

            // lua_getmetatable��ȡ����metatable�Ļ�������������ջpushһ��nil
            if (!lua_getmetatable(L, -1))
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
        static int push_global(lua_State* L, const T* obj, const char* name,
            bool gc = false)
        {
            if (0 != push(L, obj, gc)) return -1;

            lua_setglobal(L, name);
            return 0;
        }

        template <auto fp> void def(const char* name)
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
        static T* class_constructor_caller(lua_State* L,
            const std::index_sequence<I...>&)
        {
            return new T(lua_to_cpp<Args>(L, 2 + I)...);
        }

        template <typename... Args> static int class_constructor(lua_State* L)
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

        // ��������
        static void weaktable(lua_State* L, const char* mode)
        {
            lua_newtable(L);
            lua_pushvalue(L, -1); // table is its own metatable
            lua_setmetatable(L, -2);
            lua_pushliteral(L, "__mode");
            lua_pushstring(L, mode);
            lua_settable(L, -3); // metatable.__mode = mode
        }

        // ����������
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

        template <auto pf> static int fun_thunk(lua_State* L)
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

    // ���ڱ�֤��������ǰ��Lua��ջ�Ǹɾ���
    class StackChecker
    {
    public:
        StackChecker(lua_State* L)
        {
            _L = L;
            assert(0 == lua_gettop(L));
        }
        ~StackChecker()
        {
            assert(0 == lua_gettop(_L));
        }

    private:
        lua_State* _L;
    };


    // ǰ������
    template <typename T> struct class_remove;
    // �ػ�Ϊstatic������ȫ�ֺ���
    template <typename Ret, typename... Args> struct class_remove<Ret(*)(Args...)>
    {
        using type = Ret(*)(Args...);
    };
    // �ػ�Ϊ��Ա����
    template <typename T, typename Ret, typename... Args>
    struct class_remove<Ret(T::*)(Args...)>
    {
        using type = Ret(*)(Args...);
    };
    // �ػ�Ϊconst��Ա����
    template <typename T, typename Ret, typename... Args>
    struct class_remove<Ret(T::*)(Args...) const>
        : public class_remove<Ret(T::*)(Args...)>
    {
    };

    template <typename T>
    inline constexpr bool is_lua_func =
        std::is_same<typename class_remove<T>::type, lua_CFunction>::value;

    template <auto fp,
        typename = std::enable_if_t<!std::is_same_v<decltype(fp), lua_CFunction>>>
    void reg_global_func(lua_State* L, const char* name)
    {
        lua_register(L, name, Register<decltype(fp)>::template reg<fp>);
    }

    template <lua_CFunction fp> void reg_global_func(lua_State* L, const char* name)
    {
        lua_register(L, name, fp);
    }

#undef luaL_checkis
} // namespace lcpp
template <class T> const char* lcpp::Class<T>::_class_name = nullptr;
