cmake_minimum_required(VERSION 3.16)
set (CMAKE_C_COMPILER "clang")
set (CMAKE_CXX_COMPILER "clang++")

project(ksg-clang)

# ksg-llvm-pass
add_library(ksg-llvm-pass MODULE
    lib/ksg-llvm-pass.so.cc
    )

find_package(LLVM REQUIRED CONFIG)
if(NOT LLVM_FOUND)
    message (FATAL_ERROR "[-] Canno find LLVM, try \" sudo apt install llvm \" !")
endif()

target_include_directories(ksg-llvm-pass 
    PRIVATE ${LLVM_INCLUDE_DIRS}
    )

# ksg-stack-guard
add_library(ksg-stack-guard STATIC
    lib/ksg-stack-guard.c
    )

# ksg-clang
add_executable(ksg-clang ksg-clang.c)

# For install

install(TARGETS ksg-clang
    DESTINATION bin
    )

install(TARGETS ksg-llvm-pass
    DESTINATION lib
    )

install(TARGETS ksg-stack-guard 
    DESTINATION lib
    )
