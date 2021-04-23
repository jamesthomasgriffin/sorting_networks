cmake_minimum_required(VERSION 3.10)

project(Web-Generator VERSION 0.1)

add_executable(Generator generator.cpp)

set_property(TARGET Generator PROPERTY CXX_STANDARD 17)

include_directories(Generator PRIVATE
    ../../include
    ../../include/vectorclass
    ../../include/static-sort
    )

# set(CMAKE_EXECUTABLE_SUFFIX ".html")
