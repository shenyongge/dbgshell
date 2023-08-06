#include <stdio.h>
#include <cstdint>
#include <string>
#include <map>
#include "symbol.h"
#include "maps_attr.h"
#include "logger.h"
#include "elfio/elfio.hpp"

using proc_map = std::map<std::string, maps_attr>;

using symbol_map = std::map<std::string, symbol_attr>;

using namespace ELFIO;

const char *def_path = "/proc/self/maps";

uint32_t parse_maps(const char *path, proc_map &map)
{
    char line[1024];
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
        log_error("Error opening file: %s\n", path);
        return(-1);
    }
    maps_attr attr;
    while(fgets(line, 1024, fp) != nullptr) {
        uint32_t rlt = attr.parse_line(line);
        if (rlt != 0) {
            continue;
        }
        if(attr.is_anonymous()) {
            continue;
        }
        const std::string &name = attr.get_path();
        auto ret = map.emplace(name, attr);
        if (ret.second) {
            continue;
        } else {
            auto &old = ret.first->second;
            if (old.start_addr() > attr.start_addr()) {
                old = attr;
            }
        }
    }
    fclose(fp);
    return 0;
}

uint32_t parse_symbol(const std::string &elf_path, maps_attr &attr, symbol_map &sym_map)
{
    // Create an elfio reader
    elfio reader;

    // Load ELF data
    if (!reader.load(elf_path.c_str()) ) {
        log_error("Can't find or process ELF file %s!\n", elf_path.c_str());
        return 2;
    }

    uint16_t type = reader.get_type();
    log_error("ELF file %s type is %d(0:ET_NONE,1:ET_REL,2:ET_EXEC,3:ET_DYN,4:ET_CORE).\n", 
        elf_path.c_str(), type);
    if (type != ET_EXEC && type != ET_DYN) {
        return 0;
    }

    uint64_t base_addr = 0;
    if (type == ET_DYN) {
        base_addr = attr.start_addr();
    }
    log_info("%s base addr is 0x%lx!\n", elf_path.c_str(), base_addr);

    Elf_Half sec_num = reader.sections.size();
    for ( int i = 0; i < sec_num; ++i ) {
        section* psec = reader.sections[i];
        // Check section type
        if ( psec->get_type() == SHT_SYMTAB || psec->get_type() == SHT_DYNSYM) {
            const symbol_section_accessor symbols( reader, psec );
            for ( unsigned int j = 0; j < symbols.get_symbols_num(); ++j ) {
                std::string   name;
                Elf64_Addr    value;
                Elf_Xword     size;
                unsigned char bind;
                unsigned char type;
                Elf_Half      section_index;
                unsigned char other;

                // Read symbol properties
                symbols.get_symbol(j, name, value, size, bind, type, section_index, other);
                
                if (section_index == SHN_UNDEF) {
                    continue;
                }

                if (type != STT_FUNC && type != STT_OBJECT) {
                    continue;
                }
                symbol_attr sym;
                sym.addr_base = base_addr;
                sym.addr_off = value;
                sym.type = type;
                sym.size = size;
                sym_map.emplace(name, sym);

                // log_info("add symbol: %s:0x%lx(0x%lx + 0x%lx)\n", name.c_str(), 
                //    (sym.addr_base + sym.addr_off), sym.addr_base, sym.addr_off);
            }
        }
    }
    return 0;
}

uint32_t parse_all_symbol(const char *maps_path, symbol_map &sym_map)
{
    proc_map map;
    uint32_t ret = parse_maps(maps_path, map);
    if (ret != 0) {
        return ret;
    }
    for(auto &item : map) {
        (void)parse_symbol(item.first, item.second, sym_map);
    }
    return 0;
}
const char *libc_name = "libc-?.??.so";

int get_fn_pos(const char *path)
{
    int pos = -1;
    for (int i = 0; path[i] != 0; i++) {
        if (path[i] == '/') {
            pos = i;
        }
    }
    return pos;
}

bool match_file_name(const char *name, const char *match)
{
    for (int i = 0; ; i++) {
        if (name[i] == 0 || match[i] == 0) {
            return name[i] == match[i];
        }
        if (name[i] == match[i] || match[i] == '?') {
            continue;
        }
        return false;
    }
    return false;
}

uint64_t get_so_base(uint32_t pid, const char *so_name)
{
    proc_map map;
    char path[256] = {0};
    snprintf(path, 256, "/proc/%d/maps", pid);
    uint32_t ret = parse_maps(path, map);
    if (ret != 0) {
        return ret;
    }
    for(auto &item : map) {
        const char *so_path = item.first.c_str();
        int pos = get_fn_pos(so_path);
        if (pos < 0) {
            continue;
        }
        if (match_file_name(&so_path[pos + 1], so_name)) {
            return item.second.start_addr();
        }
    }
    return 0;
}

uint64_t get_libc_base(uint32_t pid)
{
    return get_so_base(pid, libc_name);
}

symbol_map sym_map;

uint32_t colect_process_symbol(uint32_t pid)
{
    char path[128] = {0};
    snprintf(path, 256, "/proc/%d/maps", pid);
    return parse_all_symbol(path, sym_map);
}

uint32_t get_symbol_info(const char *name, symbol_attr &attr)
{
    auto iter = sym_map.find(name);
    if (iter != sym_map.end()) {
        attr = iter->second;
        return 0;
    }
    return -1;
}

uint64_t get_sym_addr(const char *name)
{
    symbol_attr attr;
    uint32_t ret = get_symbol_info(name, attr);
    if (ret != 0) {
        log_error("%s not found!\n", name);
        return 0;
    };
    uint64_t addr = attr.addr_base + attr.addr_off;
    log_info("%s address is 0x%llx (0x%llx + 0x%llx)!\n", name, 
        addr, attr.addr_base, attr.addr_off);
    return addr;
}
