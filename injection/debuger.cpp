#include "debuger.h"
#include "logger.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>

void printf_regs(const user_regs &regs)
{
    log_info("     r15   0x%016llx   %lld\n", regs.r15, regs.r15);
    log_info("     r14   0x%016llx   %lld\n", regs.r14, regs.r14);
    log_info("     r13   0x%016llx   %lld\n", regs.r13, regs.r13); 

    log_info("     r12   0x%016llx   %lld\n", regs.r12, regs.r12);
    log_info("     rbp   0x%016llx   %lld\n", regs.rbp, regs.rbp);
    log_info("     rbx   0x%016llx   %lld\n", regs.rbx, regs.rbx); 

    log_info("     r11   0x%016llx   %lld\n", regs.r11, regs.r11);
    log_info("     r10   0x%016llx   %lld\n", regs.r10, regs.r10);
    log_info("     r09   0x%016llx   %lld\n", regs.r9, regs.r9); 

    log_info("     r08   0x%016llx   %lld\n", regs.r8, regs.r8);
    log_info("     rax   0x%016llx   %lld\n", regs.rax, regs.rax);
    log_info("     rcx   0x%016llx   %lld\n", regs.rcx, regs.rcx); 

    log_info("     rdx   0x%016llx   %lld\n", regs.rdx, regs.rdx);
    log_info("     rsi   0x%016llx   %lld\n", regs.rsi, regs.rsi);
    log_info("     rdi   0x%016llx   %lld\n", regs.rdi, regs.rdi); 

    log_info("orig_rax   0x%016llx   %lld\n", regs.orig_rax, regs.orig_rax);
    log_info("     rip   0x%016llx   %lld\n", regs.rip, regs.rip);
    log_info("      cs   0x%016llx   %lld\n", regs.cs, regs.cs); 

    log_info("  eflags   0x%016llx   %lld\n", regs.eflags, regs.eflags);
    log_info("     rsp   0x%016llx   %lld\n", regs.rsp, regs.rsp);
    log_info("      ss   0x%016llx   %lld\n", regs.ss, regs.ss); 

    log_info(" fs_base   0x%016llx   %lld\n", regs.fs_base, regs.fs_base);
    log_info(" gs_base   0x%016llx   %lld\n", regs.gs_base, regs.gs_base);
    log_info("      ds   0x%016llx   %lld\n", regs.ds, regs.ds); 

    log_info("      es   0x%016llx   %lld\n", regs.es, regs.es);
    log_info("      fs   0x%016llx   %lld\n", regs.fs, regs.fs);
    log_info("      gs   0x%016llx   %lld\n", regs.gs, regs.gs);
}


void debugger::attach(uint32_t pid)
{
    m_pid = pid;
    ptrace(PTRACE_ATTACH, m_pid, nullptr, nullptr);
}

void debugger::detach()
{
    ptrace(PTRACE_DETACH, m_pid, nullptr, nullptr);
    m_pid = -1;
}

uint64_t debugger::read_memory(uint64_t address)
{
    return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
}
constexpr uint32_t long_size = sizeof(long);
union u_byte_long {
    long val;
    char chars[long_size];
} data;

uint32_t debugger::read_memory(uint64_t address, char buff[], uint32_t length)
{
    u_byte_long data;
    char *laddr;
    int i, j;

    i = 0;
    j = length / long_size;
    laddr = buff;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA, m_pid, address + i * long_size, nullptr);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = length % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA, m_pid, address + i * long_size, nullptr);
        memcpy(laddr, data.chars, j);
    }
    buff[length] = '\0';
    return 0;
}
uint32_t debugger::write_memory(uint64_t address, char buff[], uint32_t length)
{
    u_byte_long data;
    char *laddr;
    int i, j;
    i = 0;
    j = length / long_size;
    laddr = buff;
    while(i < j) {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, m_pid, address + i * long_size, data.val);
        ++i;
        laddr += long_size;
    }
    j = length % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA, m_pid, address + i * long_size, nullptr);
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, m_pid, address + i * long_size, data.val);
    }
    return 0;
}

void debugger::write_memory(uint64_t address, uint64_t value)
{
    ptrace(PTRACE_POKEDATA, m_pid, address, value);
}

void debugger::get_regs(user_regs &regs)
{
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &m_regs); 
}

void debugger::set_regs(const user_regs &regs)
{
    void *p = const_cast<user_regs *>(&regs);
    if(ptrace(PTRACE_SETREGS, m_pid, nullptr, p) < 0) {
        log_error("set_regs PTRACE_SETREGS error!\n");
    }
}

void debugger::backup_regs()
{
    if(ptrace(PTRACE_GETREGS, m_pid, nullptr, &m_regs) < 0) {
        log_error("backup_regs PTRACE_GETREGS error!\n");
    }
}

void debugger::restore_regs()
{
    if(ptrace(PTRACE_SETREGS, m_pid, nullptr, &m_regs) < 0) {
        log_error("restore_regs PTRACE_SETREGS error!\n");
    }
}
void debugger::execute() 
{
    if(ptrace(PTRACE_CONT, m_pid, nullptr, nullptr) < 0 ) {
        log_error("execute PTRACE_CONT error!\n");
        return;
    }
    wait_for_signal();
}

void debugger::get_signal_info(siginfo_t &info) 
{
    ptrace(PTRACE_GETSIGINFO, m_pid, nullptr, &info);
}

void debugger::wait_for_signal() 
{
    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);

    siginfo_t siginfo;
    get_signal_info(siginfo);

    switch (siginfo.si_signo) {
    case SIGTRAP:
        switch (siginfo.si_code) {
            //one of these will be set if a breakpoint was hit
        case SI_KERNEL:
        case TRAP_BRKPT:
        {
            log_warn("Hit breakpoint\n");
            return;
        }
        //this will be set if the signal was sent by single stepping
        case TRAP_TRACE:
            return;
        default:
            log_warn("Unknown SIGTRAP code %d\n", siginfo.si_code);
            break;
        }
        break;
    case SIGSTOP:
        log_warn("process stop %d %d\n", SIGSTOP, siginfo.si_signo);
        return;
    case SIGSEGV:
        log_warn("Yay, segfault. Reason: signo %d sigcode %d\n",siginfo.si_signo, siginfo.si_code);
        break;
    default:
        log_warn("Got signal %d\n", siginfo.si_signo);
    }
    user_regs regs;
    get_regs(regs);
    printf_regs(regs);
    
}

uint64_t debugger::sys_call(uint64_t ip, uint64_t no, uint64_t arg[6])
{
    user_regs_struct reg = {0};
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &reg);
    reg.rax = no; // system call number
    reg.rip = ip; // execute ip address;
    reg.rdi = arg[0];
    reg.rsi = arg[1];
    reg.rdx = arg[2];
    reg.r10 = arg[3];
    reg.r8  = arg[4];
    reg.r9  = arg[5];
    set_regs(reg);
    execute();
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &reg);
    return reg.rax;
}

/*
CALL EAX	FF D0
CALL ECX	FF D1
CALL EDX	FF D2
CALL EBX	FF D3
CALL ESP	FF D4
CALL EBP	FF D5
CALL ESI	FF D6
CALL EDI	FF D7
*/
uint64_t debugger::func_call(const Transit &trans, uint64_t funcaddr, const ArgStru args[6])
{
    uint8_t call_rax_code[8] = {0xff, 0xd0, 0xcc, 0x90, 0x90, 0x90, 0x90, 0x90};
    write_memory(trans.addr, (char *)call_rax_code, (uint32_t)sizeof(call_rax_code)); 
    uint64_t regs[6];
    uint64_t top = trans.addr + trans.len - 0x10;
    for (int i = 6 - 1; i >= 0; i--) {
        if (args[i].type == 0) {
            regs[i] = args[i].u.atomic;
            continue;
        }
        uint32_t arglen = args[i].u.complex.len;
        void *local = args[i].u.complex.local_addr;
        top = (top - arglen) & (~(uint64_t)0xF);
        regs[i] = top;
        write_memory(top, (char *)local, arglen);
        log_info("arg[%d]: local_addr %p remote_addr %llx len %d\n", 
            i, local, regs[i], arglen);
    }

    user_regs_struct reg = {0};
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &reg);
    top = top & (~(uint64_t)0xFF);

    reg.rdi = regs[0];
    reg.rsi = regs[1];
    reg.rdx = regs[2];
    reg.rcx = regs[3];
    reg.r8  = regs[4];
    reg.r9  = regs[5];
    reg.rax = funcaddr;
    reg.rsp = top;
    reg.rbp = top;
    reg.rip = trans.addr;
    log_info("FunCall: %llx sp %llx bp %llx func %llx arg[0] %llx arg[1] %llx" 
        " arg[2] %llx arg[3] %llx arg[4] %llx arg[5] %llx\n",
        reg.rip, reg.rsp, reg.rbp, funcaddr, 
        regs[0], regs[1], regs[2], regs[3], regs[4], regs[5]);
    set_regs(reg);
    execute();
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &reg);
    log_info("return %llx\n", reg.rax);
    return reg.rax;
}
/*
struct PatchInfo {
    uint64_t orgin;
    uint64_t target;
    char code_bakup[16];
};
*/
uint32_t debugger::patch_func32(uint64_t orgin, uint64_t target)
{
    PatchInfo patch = {orgin, target, {0}};
    read_memory(orgin, patch.code_bakup, sizeof(patch.code_bakup));
    int64_t diff = target - orgin - 5;
    if (diff > INT32_MAX || diff < INT32_MIN) {
        log_error("Patch Type error! JUMP distance %lld(0x%llx - 0x%llx - 0x05) is overflow!\n", 
            diff, target, orgin);
        return -1;
    }
    int offset = (int)diff;

    uint8_t jmp_code[8] = {0xe9, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90};
    memcpy(&jmp_code[1], &offset, sizeof(offset));

    write_memory(orgin, (char*)jmp_code, sizeof(jmp_code));
    return 0;
}

uint32_t debugger::patch_func(uint64_t orgin, uint64_t target)
{
    PatchInfo patch = {orgin, target, {0}};
    read_memory(orgin, patch.code_bakup, sizeof(patch.code_bakup));

    /*
    FF25     jmp qword ptr[相对地址] offset = (u32)0
    */
    uint8_t jmp_far_code[16] = {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};   

    memcpy(&jmp_far_code[6], &target, sizeof(target));
    write_memory(orgin, (char *)jmp_far_code, (uint32_t)sizeof(jmp_far_code));  
    return 0;
}

void *debugger::remote_mmap(uint64_t pc, uint32_t size)
{
    uint8_t code_backup[16] = {0};
    read_memory(pc, (char*)code_backup, sizeof(code_backup));
    // 0x0f 0x05 syscall
    uint8_t syscall_code[8] = {0x0f, 0x05, 0xcc, 0x90, 0x90, 0x90, 0x90, 0x90};
    write_memory(pc, (char*)syscall_code, sizeof(syscall_code));

    uint64_t args[6] = {0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, (uint64_t)-1, 0};
    uint64_t no = 9; //mmap
    uint64_t ret = sys_call(pc, no, args);
 
    write_memory(pc, (char*)code_backup, sizeof(code_backup));
   return (void *)ret;
}