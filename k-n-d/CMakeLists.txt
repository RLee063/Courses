cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE "Debug")

project(kn-DNS)

if(NOT DEFINED ENV{UV_INCLUDE_DIRS})
    set(ENV{UV_INCLUDE_DIRS} "/usr/local/include")
endif()
if(NOT DEFINED ENV{UV_LIBRARY_DIRS})
    set(ENV{UV_LIBRARY_PATH} "/usr/local/lib/libuv.so")
endif()

add_executable(knDNS src/knDNS.c)

target_include_directories(knDNS
    PRIVATE $ENV{UV_INCLUDE_DIRS}
    PRIVATE ${PROJECT_SOURCE_DIR}/include
    )

target_link_libraries(knDNS
    PRIVATE $ENV{UV_LIBRARY_PATH}
    )
