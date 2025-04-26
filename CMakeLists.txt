cmake_minimum_required(VERSION 3.16)
project(make_asm_depend LANGUAGES CXX)

add_executable(make_asm_depend "src/main.cpp"
                               "src/helpers.cpp")

set_property(TARGET make_asm_depend PROPERTY CXX_STANDARD 17)
