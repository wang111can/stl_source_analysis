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

* 第二级置配器__malloc_alloc_template 避免太多小额区块造成内存碎片
> 当区块够小时，使用内存池管理: 配置一块大内存，维护对应的自由链表
> 
```cpp
enum {
    __ALIGN = 8
}; // 小区块的下界

enum {
    __MAX_BYTES = 128
}; // 小区快的上界

enum {
    __NFREELISTS = __MAX_BYTES / __ALIGN
}; // free-lists 个数

template <bool threads, int inst> // 第一参数用于多线程情况下
class __default_alloc_template {
    
    private:
    // 将bytes 上调至8的倍数
    static size_t ROUND_UP(size_t bytes) {
        return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
    }

    private:

    // 区块大小 依次为 8 16 24 32 ... 128
    union obj { // free-list
        union obj *free_list_link;
        char client_data[1];
    };

    static obj *volatile free_list[__NFREELISTS];

    // 根据区块大小 选择使用第n号free—list (1~n)
    static size_t FREELIST_INDEX(size_t bytes) {
        return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
    }

    // 返回大小为 n 的对象， 加入大小为n的其它区块到 free-list
    static void *refill(size_t n);

    // 配置最大可容纳nobjs个大小为 size 的区块 nobjs(0~nobjst)
    static char *chunck_alloc(size_t size, int &nobjs);

    static char *start_free;
    static char *end_free;
    static size_t heap_size;

    public:
    
    // 根据情况来使用 一二级配置器
    static void *allocate(size_t n) {
        obj *volatile *my_free_list;
        obj* result;
        if (n > (size_t) __MAX_BYTES) {
            return malloc_alloc::allocate(n);
        }

        my_free_list = free_list+ FREELIST_INDEX(n);
        result = *my_free_list;
        if (result == 0) {
            void *r = refill(ROUND_UP(n)); // 没找到可用的free-list 重新填装free-list
            return r;
        }

        // 从free-list[index] 中删掉获得的区块 
        // free-list[index] 为一个 指定字节大小的链表
        *my_free_list = result->free_list_link;
        return (result);

    }

    // 根据情况来使用 一二级配置器
    static void deallocate(void *p, size_t n) {
        obj *q = (obj*) p;
        obj *volatile *my_free_list;

        if (n > (size_t)__MAX_BYTES) {
            malloc_alloc::deallocate(p, n);
            return ;
        }

        my_free_list = free_list + FREELIST_INDEX(n); 
        // 把q指向的区块并入free-list[index]
        q->free_list_link = *my_free_list;
        *my_free_list = q;
    }

    static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};


// 初始化

template<bool threads, int inst> 
char *__default_alloc_template<threads, inst>::start_free = 0;

template<bool threads, int inst> 
char *__default_alloc_template<threads, inst>::end_free = 0;


template<bool threads, int inst> 
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template<bool threads, int inst>
__default_alloc_template<threads, inst>::obj* volatile __default_alloc_template<threads, inst>::free_list[__NFREELISTS] = {0};


template<bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n) { // ROUND_UP(n)
    int nobjs = 20; // 新区块的个数
    char* chunk = chunk_alloc(n, nobjs); // chunk 为一大块内存 大小为 nobjes * n
    obj* volatile *my_free_list;
    obj* result;
    obj* current_obj, * next_obj;
    
    if (1 == nobj1) return (chunk); // 如果只获得了一个区块， 直接返回该区块
    result = (obj*)chunk;
    my_free_list = free_list + FREELIST_INDEX(n);
    *my_free_list = next_obj = (obj*)(chunk + n); // 跳过准备返回的区块
    
    // 链接区块
    for (int i = 1; ;i ++ ) {
        current_obj = next_obj;
        next_obj = (obj*)((char*)next_obj + n);
        if (nobjs - 1 == i) {
            current_obj ->free_list_link = 0;
            break;
        }
        else {
            current_obj->free_list_link = next_obj;
        }
    }
    return result;
}


template<bool threads, int inst> 
char* __default_alloc_template<threads, inst>::chunck_alloc(size_t size, int& nobjs) {
    char* result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free; // 内存池剩余空间

    if (bytes_left >= total_bytes) {
        result = start_free;
        start_free += total_bytes;
        return (result);
    }
    else if (bytes_left >= size){ // 至少返回一个区块
        nobjs = bytes_left / size;
        total_bytes = size * nobjs;
        result = start_free;
        start_free += total_bytes;
        return (result);
    }
    else { // 内存池不够
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4); // heap_size / 16
        
        if (bytes_left > 0) { // 尽可能的利用剩余的内存
            obj* volatile* my_free_list = free_list + FREELIST_INDEX(bytes_left);
            ((obj*)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*)start_free;
        }
        // 配置空间
        start_free = (char*)malloc(bytes_to_get);
        if (nullptr == start_free) {
            obj* volatile* my_free_list, *p;
            for (int i = size; i <= __MAX_BYTES;i += __ALIGN) { // 尝试将free-list 中较大的区块分配
                                                                // 多余的空间终将会被重新回收利用起来
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if (nullptr != p) {
                    *my_free_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = start_free + i;
                    // 重新获取分配内存
                    return (chunck_alloc(size, nobjs));
                }
            }
            end_free = 0;
            start_free = (char*)malloc_alloc::allocate(bytes_to_get); // 尝试调用一级置配器
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return (chunck_alloc(size, nobjs));        // 重新获取分配内存
    }


}
```