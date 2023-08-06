cmake_minimum_required (VERSION 3.0)
project (gdbshell)


include_directories(3rdpart/ELFIO include)

add_subdirectory(main)
add_subdirectory(pub)
add_subdirectory(event)
add_subdirectory(symbol)
add_subdirectory(injection)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

