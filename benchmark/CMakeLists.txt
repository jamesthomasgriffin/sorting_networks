cmake_minimum_required(VERSION 3.17)

project(Benchmark)

add_executable(Benchmark benchmark.cpp)

set_property(TARGET Benchmark PROPERTY
    CXX_STANDARD 17
    )

include_directories(Benchmark PRIVATE
    ../include
    ../include/vectorclass
    ../include/static-sort
    )
