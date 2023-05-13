cmake_minimum_required(VERSION 3.15)

project(unstately VERSION 0.1.0)
set(UNSTATELY_VERSION "${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wvla -Wall -Wextra -pedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
endif()

option(UNSTATELY_BUILD_DOXYGEN "Build Doxygen documentation" YES)
option(UNSTATELY_BUILD_EXAMPLES "Build examples" YES)

add_subdirectory(src)

if(UNSTATELY_BUILD_DOXYGEN)
    add_subdirectory(docs)
endif()

if(UNSTATELY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
