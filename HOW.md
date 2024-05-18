# C++与Lua交互

# 现代化的C++(parameter pack)

# 重载与可变参数
简单的事情简单处理，复杂的事情特殊处理

# 构造函数
1. `__call`
2. 构造函数指针
3. 单例(`std::is_constructible`)

# 安全
### 类型安全
### long jump

# 已知问题
### 引用不能用作参数
当我们尝试把`void test3(std::string& i)`这样的函数注册到Lua时，会出现`'Register<void (*)(std::string &)>::reg<&test3>'`这种报错，原因是模板参数类型是`std::string &`，找不到合适的模板函数。
这个简单，那我们特化这个类型的实现`std::string &lua_to_cpp<(std::string&>(lua_State *L, int index)`对吧。

可是再仔细看看这函数，返回了一个引用，这不太对啊。改成`std::string lua_to_cpp<(std::string&>(lua_State *L, int index)`吧，可是还是编译不过啊。
`candidate template ignored: could not match 'std::string &(lua_State *, int)' against 'std::string (lua_State *, int)' (aka 'basic_string<char, char_traits<char>, allocator<char>> (lua_State *, int)')`


为什么呢？因为我们的模板函数是`T lua_to_cpp(lua_State *L, int index)`，注意它的返回值和模板参数是一样的，而上面的写法，模板参数是`std::string&`返回值是`std::string&`，这个明显就对不上了。

针对这个问题，我们可以用`std::remove_reference_t`来解决，`std::remove_reference_t<T> lua_to_cpp(lua_State *L, int index)`就样就允许模板参数是引用，返回值是原类型，上面的代码就可以实现了。

可是再想想，这个做法不太好啊。除了`std::string`这个类型，还有int、double等待类型，除了引用，还有右值引用，还有const，每一个类型都要实现几个版本吗？这不得累死。
Lua传参数到C++，是通过堆栈传的，这些参数在C++都是不可修改的，即都是const，并且不可能是引用的。所以这里我们只要知道参数是什么类型，而不需要知道这个参数是const还是引用又或者是右值引用。

所以这里直接一了百了，把传过来的参数，把const和引用、右值引用全部去掉。那怎么做呢，C++ 20有一个`std::remove_cvref`，但我用的是C++17，需要自己处理一下：
```cpp
template <typename T>
using remove_cvref = std::remove_cv<std::remove_reference_t<T>>::template type;

static int caller(lua_State* L, Ret(*fp)(Args...), const std::index_sequence<I...>&)
{
    // 调用的时候先把引用去掉
    cpp_to_lua(L, fp(lua_to_cpp<remove_cvref<Args>>(L, 1 + I)...));
    return 1;
}
```
但是这里请注意啊，`lua_to_cpp<remove_cvref<Args>>`只是把传入的参数去掉引用，但返回值没去掉。`const char* lua_to_cpp<const char*>(lua_State* L, int i)`和`char* lua_to_cpp<char*>(lua_State* L, int i)`这两个函数因为返回值不一样，还是会根据fp的参数类型选择合适的类型的。

但是，这样就解决问题了吗？在VS 2022下，确实编译通过了，运行也没有发现问题。如果不是我在另一个地方做了测试，还不清楚有这个问题。

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

另外，在查`std::remove_cvr`的资料时，我发现在一起的还有一个`std::remove_volatile`，不过这个我没用过，暂时就不处理了。


### `void dump() const`不能注册

上面解决了参数的const问题，但是函数也有const，比如`int get_i() const`，这样也需要一个const类型才能处理。最简单的办法当然是多实现一套const的模板，但是上面用`std::remove_cv`去掉了，这里我也想直接去掉，因为一个const函数只是编译时起作用，对于Lua运行时调用这个const并没有什么使用。
结果出乎意料的是`std::remove_cv`对函数类型是不生效的。

哪咋办呢，实现两套模板？我原本想的是，只有一个ClassRegister类，里面多实现一个带const的caller函数即可。结果发现不行，因为函数类型是传给ClassRegister类的，那就要实现一套`class ClassRegister<Ret(T::*)(Args...)>`，另一套`class ClassRegister<Ret(T::*)(Args...) const>`。
这样是可以的，但是这两套模板，除了这个const其他完全一样，没达到代码复用的目的，比较难维护。

复用代码这个事嘛，一个是组合，一个是继承，宏定义这种就不用了。那这里用继承比较合适，用带const的去继承不带const的即可，里面的函数就可以不用重新实现一套。
```cpp
    template<typename Ret, typename... Args>
    class ClassRegister<Ret(T::*)(Args...) const> : public ClassRegister<Ret(T::*)(Args...)>
```
只是原来的caller函数是带类型的`static int caller(lua_State* L, Ret(T::* fp)(Args...), const std::index_sequence<I...>&)`，那现在这个类型就不固定了，因此也要改成和类型无关的auto，并放到模板参数中
```cpp
        template <auto fp, size_t... I>
        static int caller(lua_State* L, const std::index_sequence<I...>&)
```

我又查了一下，发现[https://stackoverflow.com/questions/38767993/should-stdremove-cv-work-on-function-types](https://stackoverflow.com/questions/38767993/should-stdremove-cv-work-on-function-types)这里是可以把函数的const去掉的

```cpp
// 构造函数指针不可获取，因为不知道是哪个重载
// 单例不允许创建
// 默认无参数构造函数
// 手动指定参数
/*
* lclass<Map> lc("Engine.Map"); // 默认无参数构造函数
* lclass<Map> lc<int, std::string>("Engine.Map"); // 指定构造参数
* lclass<Map> lc("Engine.map", nullptr); // 指定构造参数指针（nullptr为不允许构造）
* 
* 重载问题 enable_if
* 安全问题
*/
```

