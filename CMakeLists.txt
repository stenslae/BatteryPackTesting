cmake_minimum_required(VERSION 3.15)

project(test CXX)

add_executable(test ctesting.cpp)
add_executable(battery Battery_Project.cpp)

target_link_directories(test PRIVATE "C:\\Program Files (x86)\\LabJack\\Drivers\\64bit")
target_link_libraries(test PRIVATE LabJackM)
target_include_directories(test PRIVATE "C:\\Program Files (x86)\\LabJack\\Drivers")
target_compile_options(test PRIVATE "-Wall")

target_link_directories(battery PRIVATE "C:\\Program Files (x86)\\LabJack\\Drivers\\64bit")
target_link_libraries(battery PRIVATE LabJackM)
target_include_directories(battery PRIVATE "C:\\Program Files (x86)\\LabJack\\Drivers")