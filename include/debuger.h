#ifndef __DEBUGGER__
#define __DEBUGGER__

#include <cstdint>
#include <sys/user.h>
#include <signal.h>

using user_regs = user_regs_struct;

struct ArgStru {
    struct Complex {
        void *local_addr;
        uint32_t len;
    };
    union U
    {
        uint64_t atomic;
        Complex complex;
    } u;
    uint32_t type; // 0 atomic, 1 complex
};

struct Transit // a place for temp
{
    uint64_t addr;
    uint32_t len;
};

struct PatchInfo {
    uint64_t orgin;
    uint64_t target;
    char code_bakup[16];
};

class debugger {
public:
    void attach(uint32_t pid);
    void detach();
    uint64_t read_memory(uint64_t address);
    void write_memory(uint64_t address, uint64_t value);
    void get_regs(user_regs &regs);
    void set_regs(const user_regs &regs);
    void execute();
    void wait_for_signal();
    void backup_regs();
    void restore_regs();
    void get_signal_info(siginfo_t &info); 

    uint64_t func_call(const Transit &trans, uint64_t funcaddr, const ArgStru args[6]);
    uint32_t read_memory(uint64_t address, char buff[], uint32_t length);
    uint32_t write_memory(uint64_t address, char buff[], uint32_t length);
    uint32_t patch_func32(uint64_t orgin, uint64_t target);
    uint32_t patch_func(uint64_t orgin, uint64_t target);

    void *remote_mmap(uint64_t pc, uint32_t size);
private:
    uint64_t sys_call(uint64_t ip, uint64_t no, uint64_t arg[6]);
private:
    uint32_t m_pid;
    user_regs_struct m_regs;
};


#endif