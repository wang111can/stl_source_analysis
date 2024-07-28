#include <iostream>

template <class T>
struct testclass {
    testclass() {
        std::cout << "Generalized design" << std::endl;
    }
};
template<> 

struct testclass<char> {
    testclass() {
        std::cout << "Specialized design for char" << std::endl;
    }
};

template<> 
struct testclass<unsigned char> {
    testclass() {
        std::cout << "Specialized design for unsgined char" << std::endl;
    }
};

template <class T> 
struct testclass<T*>{
        testclass() {
        std::cout << "Specialized design for pointer" << std::endl;
    }
};

//overloaded operator() makes add a functor
template<class T> 
struct add {
    T operator() (const T& a, const T& b) const {
        return a - b;
    }
};

int main() {

    testclass<int> a;
    testclass<char> b;
    testclass<unsigned char> c;
    testclass<int*> d;

    add<int> add_obj;
    std::cout << add_obj(5, 4) << std::endl; 

    return 0;

}