#ifndef __MAPS_ATTR__
#define __MAPS_ATTR__

#include <stdlib.h>
#include <string>

constexpr uint32_t MAPS_PERM_READABLE = 0x1;
constexpr uint32_t MAPS_PERM_WRITEABLE = 0x2;
constexpr uint32_t MAPS_PERM_EXECUTEABLE = 0x4;
constexpr uint32_t MAPS_PERM_SHARED = 0x8;
constexpr uint32_t MAPS_PERM_PRIVATED = 0x16;

class maps_attr
{
  uint64_t start_address;
  uint64_t end_address;
  uint64_t offset;
  uint32_t dev_major;
  uint32_t dev_minor;
  ino_t inode;
  uint8_t permissions;
  std::string path;

public:
  uint32_t parse_line(char *unparsed_line);
  // getters
  uint64_t start_addr()
  {
    return start_address;
  }
  uint64_t end_addr()
  {
    return end_address;
  }
  const std::string &get_path() const
  {
      return path;
  }

  size_t length();

  // getters for the permissions bitmask
  uint16_t perms()
  {
    return permissions;
  }

  bool is_bindable();
  bool is_anonymous();
  bool is_heap();
  bool is_stack();
  void print();
};

#endif