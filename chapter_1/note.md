
```cpp
/*
stl class partial specialization 
*/

#include <iostream>

// Generalized design
template <class A, class B>
struct  testclass {
    
    testclass () {
        std::cout << "Generalized design" << std::endl;
    }
};

//Specialized design
template <class A, class B>
struct testclass<A* , B* > {
    testclass () {
        std::cout << "Specialized design" << std::endl;
    }
};

//Explicit specialized design
template<>
struct testclass<char, int> {
    testclass() {
        std::cout << "Specialized design for char and int" << std::endl;
    }
}


//overloaded operator() makes add a functor
template<class T> 
struct add {
    T operator() (const T& a, const T& b) const {
        return a - b;
    }
};


int main() {
    testclass<int, int> a;
    testclass<int*, int*> b;
    testclass<int, char> c;
    testclass<int*, char*> d;
    testclass<char, int> e;

    add<int> add_obj;
    std::cout << add_obj(5, 4) << std::endl; 
    
    return 0;

}
```

