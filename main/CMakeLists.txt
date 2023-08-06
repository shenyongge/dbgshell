aux_source_directory(. MAIN_SRC)

add_executable(gdbshell  main.cpp
    $<TARGET_OBJECTS:pub> 
    $<TARGET_OBJECTS:event>
    $<TARGET_OBJECTS:symbol>
    )

target_link_libraries(gdbshell PUBLIC -lutil) 


add_executable(injecter  injecter.cpp
    $<TARGET_OBJECTS:pub> 
    $<TARGET_OBJECTS:symbol>
    $<TARGET_OBJECTS:inject>
    )

target_link_libraries(gdbshell PUBLIC -lutil) 

add_executable(dummy  dummy.cpp)
#target_link_libraries(dummy PUBLIC -L. -lshareobj2 -Wl,-rpath,./ ) 
target_link_libraries(dummy PUBLIC -ldl -Wl,-rpath,./ ) 

add_library(shareobj SHARED shareobj.cpp)
add_library(shareobj2 SHARED shareobj2.cpp)