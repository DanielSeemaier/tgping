cmake_minimum_required(VERSION 3.16)  
project(tgping)              
set(CMAKE_CXX_STANDARD 17)          

find_package(CURL REQUIRED)

add_executable(tgping tgping.cpp telegram.cpp)
target_link_libraries(tgping PRIVATE CURL::libcurl)

