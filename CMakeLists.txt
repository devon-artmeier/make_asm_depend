cmake_minimum_required(VERSION 3.16)
project(make-asm-dependencies LANGUAGES CXX)

add_executable(make-asm-dependencies "source/main.cpp")

set_property(TARGET make-asm-dependencies PROPERTY CXX_STANDARD 17)
