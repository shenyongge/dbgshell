#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <stdint.h>

class Test 
{
public:
    Test()
    {
        printf("so is opening and Test Construct is called!\n");
    }
    ~Test()
    {
        printf("Test DeConstruct is called and so is closing !\n");
    }
};
Test test;

extern "C" uint32_t MySub(uint32_t x, uint32_t y)
{
    printf("MySub is called: %d - %d = %d!\n", x, y, x - y);
    return x - y;
}

