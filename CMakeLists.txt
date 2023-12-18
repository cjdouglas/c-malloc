cmake_minimum_required(VERSION 3.25)

project("cmemory")

set(SOURCES src/cmemory.c main.c)
add_executable(cmemory ${SOURCES})
target_include_directories(cmemory PRIVATE include)
set_target_properties(cmemory PROPERTIES C_STANDARD 11)
