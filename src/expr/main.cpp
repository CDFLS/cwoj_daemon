//
// Created by zhangyutong926 on 10/25/16.
//

#include <iostream>
#include <string>

using namespace std;

template<typename B, typename T>
inline T *FromOffset(B *base, T B:: *offset) {
    unsigned char *baseInter = (unsigned char *) base;
    int offsetValue = reinterpret_cast<int>(*(void **)(&offset));
    baseInter += offsetValue;
    return reinterpret_cast<T *>(baseInter);
}

class A {
public:
    int b = 1;
    string c = "All hail meta-programming!";
};

int main(int argc, char **argv) {
    A *a = new A();
    cout << *FromOffset<A, int>(a, &A::b) << std::endl;
    cout << *FromOffset<A, string>(a, &A::c) << std::endl;
}
