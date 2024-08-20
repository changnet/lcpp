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

复用代码这个事嘛，一个是组合，一个是继承，宏定义这种就不用了。那这里用继承比较合适，用带const的去继承不带const的即可，里面的函数就可以不用重新实现一套，全部继承就行。
```cpp
    template<typename Ret, typename... Args>
    class ClassRegister<Ret(T::*)(Args...) const> : public ClassRegister<Ret(T::*)(Args...)>
    {};
```
只是原来的caller函数是带类型的`static int caller(lua_State* L, Ret(T::* fp)(Args...), const std::index_sequence<I...>&)`，那现在这个类型就不固定了，因此也要改成和类型无关的auto，并放到模板参数中
```cpp
        template <auto fp, size_t... I>
        static int caller(lua_State* L, const std::index_sequence<I...>&)
```

我又查了一下，发现[https://stackoverflow.com/questions/38767993/should-stdremove-cv-work-on-function-types](https://stackoverflow.com/questions/38767993/should-stdremove-cv-work-on-function-types)这里是可以把函数的const去掉的。
不过我已经用上面的方法实现了，这个暂且做个备忘吧。

### 构造函数
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
我原本是把构造函数放到类的模板参数里的。

```cpp
LClass<T1, int, double> t1("T1");
```
一开始我觉得这种设计还挺好的，在定义一个类的时候顺便也定义了它的构造参数。

但我在使用的时候，发现一个比较麻烦的问题。假如定义了一个类为T1，那现在需要用它，就要用到原的模板。假如
```cpp
// 定义一个类
LClass<T1, int, double> t1("T1");

// 在另一个地方，使用这个类
LClass<T1, int, double>::push(L, t);
```
这下问题就来了，使用的时候，如何记得T1这个类有几个构造参数呢？稍微不小心就会写错，这不是在为难写代码的人么。所以想了想，我改成这样
```cpp
// 定义一个类，给一个默认的构造函数
LClass<T1> t1("T1");
// 如果需要调用某个特别的构造函数，需要手动指定构造参数
t1.constructor<int, double>();

// 在另一个地方，使用这个类
LClass<T1>::push(L, t);
```
这样使用的时候只有类名，像`LClass<T1>`，这下不可能会传错参数了吧。

# 继承
使用的时候，又发现了一个问题。派生类可以调用基类的函数，当然也有在lua调用基类函数的需求。可是却注册不了基类的函数。
```
class A
{
public:
	void test();
};

class B
{
public:
	void do_other();
};

LClass<B> lc;
lc.def<&A::test()>("test"); // 出错
```

原因是`ClassRegister`把函数类型(T::*)限制死了，考虑去掉T或者把T这个类型放到函数里，然后用`std::is_base_of`保证是基类函数即可。

这里还有个问题，之前是通过`typename = std::enable_if_t<!std::is_same_v<decltype(fp), lua_CppFunction>`来判断的，现在lua_CppFunction中的T不一样了，那这个判断也不能放这里了，得放`ClassRegister`

如此就可以实现把基类的函数在派生类中定义到Lua

# static成员函数
在C++里，static成员函数可以通过类名来调用，也可能通过对象来调用。通过对象来调用时，和调用普通函数是一样的
```cpp
class Test
{
public:
    static void static_func() {}
    void none_static_func() {}
};

Test t;
t.static_func();
t.none_static_func();
```
但是在Lua里，这却是完全不一样的
```lua
local t = Test()

t.satic_func()
t:none_static_func()
```
那写代码的人，很可能留意不到这个区别，或者说如果写代码的人要时刻关注一个函数要采用哪种做法，那这个底层的设计是比较失败的。所以我干脆把static函数，也按普通函数来处理，第一个参数也需要一个对象来占位即可。

当然因为它是static函数，显然也有直接通过类来调用而不仅仅是对象的需求，比如`Test.static_func`。当这个函数有参数时，这就很头疼，因为Lua的`Test:static_func`和`Test.static_func`这两种调用方式，在底层取参数是完全不一样的。

那这里定义一下规则
1. 如果底层是int (*)(lua_State *)函数的形式，则参数由对应的函数写法来定，这个模块不做处理
2. 如果是其他函数，则必须按对象方式来调用，比如通过类直接调用时为`Test:static_func`

我想过在C++判断第一个参数是否为当前类的指针，如果是的话则判定为对象方式调用，则忽略第一个参数。但是这种方式就没法识别参数恰好是对象指针的情况
```cpp
class Test
{
public:
    // 这种函数，参数恰好是当前类的指针。Test.static_func(t)这样调用时，参数会被忽略
    static void static_func(Test *t)
    {}
}
```
所以第一个参数统一忽略吧，这样还方便些。可以`t:static_func()`调用，也可以`Test:static_func()`来调用，也可以`Test.static_func(nil, ...)`来调用。但有参数时，请不要`Test.static_func(...)`这样来调用。
