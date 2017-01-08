cmake_minimum_required(VERSION 2.6)
project(libble)

add_executable(libble main.cpp)

install(TARGETS libble RUNTIME DESTINATION bin)
