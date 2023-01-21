cmake_minimum_required(VERSION 3.25.1)

set(CMAKE_C_COMPILER_WORKS true)
set(CMAKE_CXX_COMPILER_WORKS true)
set(CMAKE_C_COMPILER "clang.exe")
set(CMAKE_CXX_COMPILER "clang++.exe")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(calculator)

set(root_source "${PROJECT_SOURCE_DIR}/source/")

message(${root_source})

include_directories(${root_source})

add_executable(calculator "${root_source}calculator.cpp")

# this is supposed to prevent vscode from cutting off error messages in its problems window
add_compile_options("-fmessage-length=0")