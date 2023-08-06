#include "maps_attr.h"
#include "logger.h"
#include <string.h>


uint32_t maps_attr::parse_line(char *line) {
    int name_start = 0, name_end = 0;
    uint64_t addr_start, addr_end;
    char perms_str[8];
    log_info("line: %s", line);

    if(sscanf(line, "%lx-%lx %7s %lx %u:%u %lu %n%*[^\n]%n",
                     &addr_start, &addr_end, perms_str, &offset,
                     &dev_major, &dev_minor, &inode,
                     &name_start, &name_end) < 7) {
      log_error("line: %s", line);
      return -1;
    }

    // convert addresses
    start_address = addr_start;
    end_address = addr_end;

    // copy permissions
    permissions = 0U;
    if (strchr(perms_str, 'r')) {
        permissions |= 1U << 0;
    }
    if (strchr(perms_str, 'w')) {
        permissions |= 1U << 1;
    }

    if (strchr(perms_str, 'x')) {
        permissions |= 1U << 2;
    }
    if (strchr(perms_str, 's')) {
        permissions |= 1U << 3;
    }
    if (strchr(perms_str, 'p')) {
        permissions |= 1U << 4;
    }


    // copy name
    if (name_end > name_start) {
        line[name_end] = '\0';
        path.assign(&line[name_start]);
    } else {
        path.clear();
    }
    return 0;
}

size_t maps_attr::length() {
  return ((char*)end_address) - ((char*)start_address);
}

void maps_attr::print() {
  printf("[%18lx-%18lx] [off=%7lu] [dev=%u:%u] [inode=%8lu] %c%c%c%c '%s'",
           start_address, end_address,
           offset,
           dev_major, dev_minor,
           inode,
           ((permissions & MAPS_PERM_READABLE) ?    'R' : '-'),
           ((permissions & MAPS_PERM_WRITEABLE) ?   'W' : '-'),           
           ((permissions & MAPS_PERM_EXECUTEABLE) ? 'X' : '-'),
           ((permissions & MAPS_PERM_PRIVATED) ?    'P' : 'S'),
           path.c_str());
}

bool maps_attr::is_bindable() {
  return get_path() != "[vsyscall]";
}

bool maps_attr::is_heap() {
  return get_path() == "[heap]";
}

bool maps_attr::is_stack() {
  return get_path() == "[stack]";
}

bool maps_attr::is_anonymous() {
  return get_path().length() == 0;
}