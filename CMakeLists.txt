cmake_minimum_required(VERSION 3.29)
project(arm_rule_compression)

set(CMAKE_CXX_STANDARD 20)

add_executable(arm_rule_compression src/main.cpp src/predicate.cpp src/parser.cpp src/compression.cpp)
