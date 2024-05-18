# C++��Lua����

# �ִ�����C++(parameter pack)

# ������ɱ����
�򵥵�����򵥴������ӵ��������⴦��

# ���캯��
1. `__call`
2. ���캯��ָ��
3. ����(`std::is_constructible`)

# ��ȫ
### ���Ͱ�ȫ
### long jump

# ��֪����
### ���ò�����������
�����ǳ��԰�`void test3(std::string& i)`�����ĺ���ע�ᵽLuaʱ�������`'Register<void (*)(std::string &)>::reg<&test3>'`���ֱ���ԭ����ģ�����������`std::string &`���Ҳ������ʵ�ģ�庯����
����򵥣��������ػ�������͵�ʵ��`std::string &lua_to_cpp<(std::string&>(lua_State *L, int index)`�԰ɡ�

��������ϸ�����⺯����������һ�����ã��ⲻ̫�԰����ĳ�`std::string lua_to_cpp<(std::string&>(lua_State *L, int index)`�ɣ����ǻ��Ǳ��벻������
`candidate template ignored: could not match 'std::string &(lua_State *, int)' against 'std::string (lua_State *, int)' (aka 'basic_string<char, char_traits<char>, allocator<char>> (lua_State *, int)')`


Ϊʲô�أ���Ϊ���ǵ�ģ�庯����`T lua_to_cpp(lua_State *L, int index)`��ע�����ķ���ֵ��ģ�������һ���ģ��������д����ģ�������`std::string&`����ֵ��`std::string&`��������ԾͶԲ����ˡ�

���������⣬���ǿ�����`std::remove_reference_t`�������`std::remove_reference_t<T> lua_to_cpp(lua_State *L, int index)`����������ģ����������ã�����ֵ��ԭ���ͣ�����Ĵ���Ϳ���ʵ���ˡ�

���������룬���������̫�ð�������`std::string`������ͣ�����int��double�ȴ����ͣ��������ã�������ֵ���ã�����const��ÿһ�����Ͷ�Ҫʵ�ּ����汾���ⲻ��������
Lua��������C++����ͨ����ջ���ģ���Щ������C++���ǲ����޸ĵģ�������const�����Ҳ����������õġ�������������ֻҪ֪��������ʲô���ͣ�������Ҫ֪�����������const���������ֻ�������ֵ���á�

��������ֱ��һ�˰��ˣ��Ѵ������Ĳ�������const�����á���ֵ����ȫ��ȥ��������ô���أ�C++ 20��һ��`std::remove_cvref`�������õ���C++17����Ҫ�Լ�����һ�£�
```cpp
template <typename T>
using remove_cvref = std::remove_cv<std::remove_reference_t<T>>::template type;

static int caller(lua_State* L, Ret(*fp)(Args...), const std::index_sequence<I...>&)
{
    // ���õ�ʱ���Ȱ�����ȥ��
    cpp_to_lua(L, fp(lua_to_cpp<remove_cvref<Args>>(L, 1 + I)...));
    return 1;
}
```
����������ע�Ⱑ��`lua_to_cpp<remove_cvref<Args>>`ֻ�ǰѴ���Ĳ���ȥ�����ã�������ֵûȥ����`const char* lua_to_cpp<const char*>(lua_State* L, int i)`��`char* lua_to_cpp<char*>(lua_State* L, int i)`������������Ϊ����ֵ��һ�������ǻ����fp�Ĳ�������ѡ����ʵ����͵ġ�

���ǣ������ͽ������������VS 2022�£�ȷʵ����ͨ���ˣ�����Ҳû�з������⡣�������������һ���ط����˲��ԣ����������������⡣

```cpp
void test3(std::string&& i)
{
    std::cout << "test3 " << i << std::endl;
}

std::string fc_ret()
{
    return "";
}

test3(fc_ret());
```

���⣬�ڲ�`std::remove_cvr`������ʱ���ҷ�����һ��Ļ���һ��`std::remove_volatile`�����������û�ù�����ʱ�Ͳ������ˡ�


### `void dump() const`����ע��

�������˲�����const���⣬���Ǻ���Ҳ��const������`int get_i() const`������Ҳ��Ҫһ��const���Ͳ��ܴ�����򵥵İ취��Ȼ�Ƕ�ʵ��һ��const��ģ�壬����������`std::remove_cv`ȥ���ˣ�������Ҳ��ֱ��ȥ������Ϊһ��const����ֻ�Ǳ���ʱ�����ã�����Lua����ʱ�������const��û��ʲôʹ�á�
����������ϵ���`std::remove_cv`�Ժ��������ǲ���Ч�ġ�

��զ���أ�ʵ������ģ�壿��ԭ������ǣ�ֻ��һ��ClassRegister�࣬�����ʵ��һ����const��caller�������ɡ�������ֲ��У���Ϊ���������Ǵ���ClassRegister��ģ��Ǿ�Ҫʵ��һ��`class ClassRegister<Ret(T::*)(Args...)>`����һ��`class ClassRegister<Ret(T::*)(Args...) const>`��
�����ǿ��Եģ�����������ģ�壬�������const������ȫһ����û�ﵽ���븴�õ�Ŀ�ģ��Ƚ���ά����

���ô���������һ������ϣ�һ���Ǽ̳У��궨�����־Ͳ����ˡ��������ü̳бȽϺ��ʣ��ô�const��ȥ�̳в���const�ļ��ɣ�����ĺ����Ϳ��Բ�������ʵ��һ�ס�
```cpp
    template<typename Ret, typename... Args>
    class ClassRegister<Ret(T::*)(Args...) const> : public ClassRegister<Ret(T::*)(Args...)>
```
ֻ��ԭ����caller�����Ǵ����͵�`static int caller(lua_State* L, Ret(T::* fp)(Args...), const std::index_sequence<I...>&)`��������������;Ͳ��̶��ˣ����ҲҪ�ĳɺ������޹ص�auto�����ŵ�ģ�������
```cpp
        template <auto fp, size_t... I>
        static int caller(lua_State* L, const std::index_sequence<I...>&)
```

���ֲ���һ�£�����[https://stackoverflow.com/questions/38767993/should-stdremove-cv-work-on-function-types](https://stackoverflow.com/questions/38767993/should-stdremove-cv-work-on-function-types)�����ǿ��԰Ѻ�����constȥ����

```cpp
// ���캯��ָ�벻�ɻ�ȡ����Ϊ��֪�����ĸ�����
// ������������
// Ĭ���޲������캯��
// �ֶ�ָ������
/*
* lclass<Map> lc("Engine.Map"); // Ĭ���޲������캯��
* lclass<Map> lc<int, std::string>("Engine.Map"); // ָ���������
* lclass<Map> lc("Engine.map", nullptr); // ָ���������ָ�루nullptrΪ�������죩
* 
* �������� enable_if
* ��ȫ����
*/
```

