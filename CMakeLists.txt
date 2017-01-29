cmake_minimum_required(VERSION 2.6)
project(libble)

include_directories(include)
add_executable(ble-test main.c src/ble.c)
target_link_libraries(ble-test m)

install(TARGETS ble-test RUNTIME DESTINATION bin)
