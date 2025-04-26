cmake_minimum_required(VERSION 3.16)
project(makeasmdep LANGUAGES CXX)

add_executable(makeasmdep
	"src/main.cpp"
	"src/helpers.cpp")

set_property(TARGET makeasmdep PROPERTY CXX_STANDARD 17)
