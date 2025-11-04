cmake_minimum_required(VERSION 3.10)

project(ticket_o)

# Include the current directory for header files 
include_directories(include)

set(CMAKE_C_STANDARD 11)

add_executable(${CMAKE_PROJECT_NAME}
   src/main.c
   src/functions.c)

target_link_libraries(${CMAKE_PROJECT_NAME}
   m)