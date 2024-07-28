# allocator


* 简易版的allocator
```cpp

#include <new>
#include <cstddef> // for ptrdiff_t(stores the distance between tow Pointers) size_t 
#include <cstdlib>
#include <climits>
#include <iostream>

namespace m_allocator {

    template<class T>
    inline T* _allocate(ptrdiff_t size, T*) {  // the size of needed memory and the data type 
        set_new_handler(0); // new 异常时返回的函数 设置为0
        T* tmp = (T*)(::operator new((size_t)(sizeof(T)*size))); // 使用全局的operator new 避免使用 局部的operator new
        if (tmp == 0) {
            std::cerr << "out of memory" << std::endl;
            exit(1);
        }
        return tmp;
    }

    template<class T>
    inline void _de_allocate(T* _ptr) {
        ::operator delete(ptr);
    }

    template<class T1, class T2>
    inline void _construct(T1 *p, const T2& value) {
        new(p) T1(value); //在指针 p 所指向的内存位置上构造一个类型为 T1 的对象，使用 value 作为构造函数的参数
    }
    
    template<class T>
    inline void _destory(T *ptr) {
        ptr->~T(); //调用了 ptr 所指向对象的析构函数 ~T()
    }


    template <class T> 
    class allocator {
        public:
            typedef T               value_type;
            typedef T*              pointer;
            typedef const T*        const_pointer;
            typedef T&              reference;
            typedef const T&        const_reference;
            typedef size_t          size_type;
            typedef ptrdiff_t       difference_type;
    

        template <class U>
        struct rebind {
            typedef allocator<U> other; // 绑定到新类型 U
        }; 

        pointer allocate(size_type n, const void *hint = 0) {
            return _allocate((difference_type) n, (pointer)0); 
            // (pointer)0 起到说明类型的作用
            // 调用 _allocate 函数分配内存
        }

        void deallocate(pointer p, size_type n) {
            _de_allocate(p);
        }

        void construct(pointer p, const_reference value) {
            _construct(p, value);
        }

        void destory(pointer p) {
            _destory(p);
        }
        
        // 相当于取地址
        pointer address(reference x) {
            return (pointer)&x;
        }

        const_pointer const_address(reference x) {
            return (const_pointer)&x;
        }
        
        size_type max_size() const {
            return size_type(UINT_MAX/sizeof(value_type));
        }
    };


}

```

* 第一级置配器__malloc_alloc_template 当剩余内存够大时使用

```cpp
#include <new>
#include <cstddef> // for ptrdiff_t(stores the distance between tow Pointers) size_t 
#include <cstdlib>
#include <climits>
#include <iostream>

#define __THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)

template<int inst> //这种模板参数使得模板能够接受一个整数作为参数，从而在编译时进行相应的实例化。
class __malloc_alloc_template {
    
    private:
    //oom: out of memory
    // 不同情况下调用不同的函数
    static void *oom_malloc(size_t);
    static void *oom_realloc(void*, size_t);
    static void (* __malloc_alloc_oom_handler)();

    public:
    
    static void* allocate(size_t n) {
        void *result = malloc(n); 
        if (0 == result) result = oom_malloc(n);
        return result;
    }

    static void deallocate(void *p, size_t) {    
        free(p);
    }

    static void* reallocate(void *p, size_t, size_t new_sz) {
        void *result = realloc(n); 
        if (0 == result) result = oom_realloc(p, new_sz);
        return result;
    }

    /*
    仿真的 set_new_handler 指定自定义的out of memory handler (因为没有使用operator new 不能直接使用set_new_handler)
    这个声明表示 __set_malloc_handler 是一个返回函数指针的函数。具体来说，它返回一个指向 void (*)() 类型函数的指针。                                     
    参数 __f 是一个指向 void (*)() 类型函数的指针
    */
    static void (* __set_malloc_handler(void (*f)()))()
    {
        void (* old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return(old);
    }
};

template <int inst> 

void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0; //默认为0

template <int inst>
void *__malloc_alloc_template<inst>::oom_malloc(size_t n) {
    void (*my_malloc_handler)();
    void *result;
    
    while(1) {
        // 不断的尝试释放 配置 
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) {
            __THROW_BAD_ALLOC;
        }
        (*my_malloc_handler)(); // 处理异常 
        result = malloc(n);
        if (result) return result;
    }
}

template <int inst>
void* __malloc_alloc_template<inst>::oom_realloc(void *p, size_t n) {
    void (*my_malloc_handler)();
    void *result;
    while (1) {
        // 不断的尝试释放 配置 
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler) {
            __THROW_BAD_ALLOC;
        }
        (*my_malloc_handler)(); // 处理异常 
        result = realloc(p, n);
        if (result) return result;
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;
```