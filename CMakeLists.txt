cmake_minimum_required(VERSION 3.16)
project(mkasmdep LANGUAGES CXX)

add_executable(mkasmdep "src/main.cpp"
                               "src/helpers.cpp")

set_property(TARGET mkasmdep PROPERTY CXX_STANDARD 17)
