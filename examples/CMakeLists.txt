cmake_minimum_required(VERSION 3.10)

project(NetworkSortExamples VERSION 0.1)

add_executable(sngenerator generator.cpp)

set_property(TARGET sngenerator PROPERTY CXX_STANDARD 17)

include_directories(sngenerator PRIVATE
    ../include
    ../include/vectorclass
    )
