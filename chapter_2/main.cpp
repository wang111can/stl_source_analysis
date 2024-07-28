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
    仿真的 set_new_handler 指定自定义的out of memory handler
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