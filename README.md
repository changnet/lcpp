# lcpp
test my idea about lua cpp binding, this repo will not update after test finish!

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
引用不能用作参数
void dump() const不能注册
