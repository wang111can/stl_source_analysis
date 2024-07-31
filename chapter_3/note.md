#迭代器

* 返回特定的template类型

```cpp
#include <iostream>
#include <string>
#include <new>
#include <memory>

template<class T>
class test {
   
    T* ptr;

public:

    typedef T value_type; 

    test(T* p): ptr(p) {}    
    
    T& operator*() const {
        return *ptr;
    }
};


template <class I>
typename I::value_type func(I x) { // 关键词typename 告诉编译器这是一个型别 因为 template 参数在编译时 未被具象化
    return *x;
}



int main() {
    test<int> t(new int(8));
    std::cout << func(t) << std::endl;
    return 0;
}
```