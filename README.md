# lcpp
test my idea about lua cpp binding, this repo will not update after test finish!

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
���ò�����������
void dump() const����ע��
