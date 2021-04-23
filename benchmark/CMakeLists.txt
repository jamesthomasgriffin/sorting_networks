cmake_minimum_required(VERSION 3.10)

project(Benchmark VERSION 0.1)

add_executable(Benchmark benchmark.cpp)

set_property(TARGET Benchmark PROPERTY CXX_STANDARD 17)

if(MSVC)
    target_compile_options(Benchmark PRIVATE "/arch:AVX2")
else()
    target_compile_options(Benchmark PRIVATE "-mavx2")
endif()

include_directories(Benchmark PRIVATE
    ../include
    ../include/vectorclass
    ../include/static-sort
    )
