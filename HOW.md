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
### `void dump() const`不能注册

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

