#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <stdint.h>

class MyClass 
{
public:
    MyClass()
    {
        printf("so is opening and MyClass Construct is called!\n");
    }
    ~MyClass()
    {
        printf("MyClass DeConstruct is called and so is closing !\n");
    }
};
MyClass test;

extern "C" uint32_t MySuber02(uint32_t x, uint32_t y)
{
    printf("Call Patch MySuber02(%d, %d)!\n", x, y);
    return x - y;
}