cmake_minimum_required(VERSION 3.25)
project(Squawk C)

set(SOURCES 
    ${CMAKE_SOURCE_DIR}/squawk.c
)

add_executable(squawk ${SOURCES})

target_compile_options(squawk PRIVATE
    $<$<CONFIG:Debug>:
        -fsanitize=address,leak,undefined 
        -pedantic 
        -Wall 
        -Wextra 
        -Wconversion 
        -Wundef 
        -Werror  
    >
)

target_link_options(squawk PRIVATE
    $<$<CONFIG:Debug>:
        -fsanitize=address,leak,undefined
    >
)

target_link_libraries(squawk PRIVATE 
    gc
    unistring
    # pcre2-posix
)
