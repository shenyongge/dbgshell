#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <stdint.h>
#include <dlfcn.h>

extern "C" uint32_t MySub(uint32_t x, uint32_t y);
typedef  uint32_t (*MYSUB)(uint32_t x, uint32_t y);

extern "C" uint32_t TestFunc(uint32_t x, uint32_t y)
{
    printf("TestFunc(%d, %d) is called\n", x, y);
    void* handle = dlopen("libshareobj.so", RTLD_NOW | RTLD_GLOBAL);
    printf("handle: %p\n", handle);
    MYSUB pSub = (MYSUB)dlsym(handle, "MyAdder2");
    pSub(x, y);
}

extern "C" uint32_t MyAdder01(uint32_t x, uint32_t y)
{
    return x + y;
}


extern "C" void ShowError()
{
    char *str = dlerror();
    if (str != nullptr) {
        printf("%s\n",str);
    }
}

extern "C" uint32_t MyAdder02(uint32_t x, uint32_t y)
{
    printf("Call patch func MyAdder02(%d, %d)\n",x, y);
    return x + y;
}

extern "C" uint32_t MySuber01(uint32_t x, uint32_t y)
{
    // printf("call patch func MySuber01(%d, %d)\n",x, y);
    return x - y;
}

extern "C" uint32_t MyDlOpen(const char *path, int mode)
{
    printf("MyDlOpen is called: path is [%s] mode is [%d]!\n", path, mode);
    return mode;
}
void *__libc_dlopen_mode (const char *name, int mode);
typedef void *(*DLOPEN)(const char *name, int mode);
int main()
{   
    int i;
    int x = 0;
    pid_t pid = getpid();
    printf("getpid: %d\n", pid);

    //DLOPEN dlp = (DLOPEN)dlsym(nullptr, "__libc_dlopen_mode");
    //printf("__libc_dlopen_mode: %p\n", dlp);
    void* handle = dlopen("libshareobj2.so", RTLD_NOW | RTLD_GLOBAL);
    //void* handle = dlp("libshareobj2.so", RTLD_NOW | RTLD_GLOBAL);
    printf("handle: %p\n", handle);
    MYSUB pSub = (MYSUB)dlsym(handle, "MySub");
    pSub(1000, 100);
    for(i = 0;i < 100000; ++i) {
        x = MyAdder01(x, 2);
        printf("My counter: %d\n", x);
        x = MySuber01(x, 1);
        sleep(5);
    }
    return 0;
}