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
### `void dump() const`����ע��

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

