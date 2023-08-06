#include "symbol.h" // tomoved
#include "debuger.h"
#include "logger.h"
#include <stdlib.h> 
#include <sys/ptrace.h> // tomoved
#include <sys/wait.h>
#include <string.h>
#include <dlfcn.h>

const char *funcname_dlopen = "__libc_dlopen_mode";
const char *funcname_startm = "__libc_start_main";
const char *funcname_test = "TestFunc";
char inject_soname[] = "libshareobj.so";
int dlopen_para1 = RTLD_LAZY | RTLD_GLOBAL;

int main(int argc, char *argv[])
{
    uint16_t pid = UINT16_MAX;

    if(argc != 2){
        return -1;
    }
    pid = atoi(argv[1]);
    colect_process_symbol(pid);

    //__dlopen
    uint64_t target_dlopen_addr = get_sym_addr(funcname_dlopen);
    uint64_t target_myopen_addr = get_sym_addr("MyDlOpen");
    uint64_t target_libc_start_addr = get_sym_addr(funcname_startm);
    uint64_t target_test_func_addr = get_sym_addr(funcname_test);

    debugger dbg;
    dbg.attach(pid);
    dbg.wait_for_signal();
    log_info("attach %d success!\n", pid);

    dbg.backup_regs();

    void *addr = dbg.remote_mmap(target_libc_start_addr, 4096 * 16);
    log_info("mmap addr is 0x%llx\n", addr);
    uint64_t mmap_addr = (uint64_t)addr;

    Transit trans = {mmap_addr, 0x4000};
    ArgStru argstu[6] = {0};
    argstu[0].type = 1;
    argstu[0].u.complex.local_addr = inject_soname;
    argstu[0].u.complex.len = (uint32_t)sizeof(inject_soname);
    argstu[1].type = 0;
    argstu[1].u.atomic = dlopen_para1;

    dbg.func_call(trans, target_dlopen_addr, argstu);
    log_info("func_call dlopen pid %d\n", pid);
    dbg.func_call(trans, target_myopen_addr, argstu);

    uint64_t target_add01_addr = get_sym_addr("MyAdder01");
    uint64_t target_add02_addr = get_sym_addr("MyAdder02");
    dbg.patch_func32(target_add01_addr, target_add02_addr);

    colect_process_symbol(pid);
    uint64_t target_sub01_addr = get_sym_addr("MySuber01");
    uint64_t target_sub02_addr = get_sym_addr("MySuber02");  
    dbg.patch_func(target_sub01_addr, target_sub02_addr);

    dbg.restore_regs();
    //dbg.execute();
    dbg.detach();
    log_info("detach pid %d\n", pid);
    return 0;
}
