cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)


project(modify_array)

include_directories(include)

add_executable(${CMAKE_PROJECT_NAME}
   src/main.c
   src/functions.c
   )