#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <cstdint>

struct symbol_attr {
    uint64_t addr_base;
    uint64_t addr_off;
    uint32_t type;
    uint32_t size;
};

uint32_t colect_process_symbol(uint32_t pid);
uint32_t get_symbol_info(const char *name, symbol_attr &attr);
uint64_t get_libc_base(uint32_t pid);
uint64_t get_so_base(uint32_t pid, const char *so_name);
uint64_t get_sym_addr(const char *name);
#endif