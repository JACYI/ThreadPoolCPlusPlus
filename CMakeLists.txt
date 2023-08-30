cmake_minimum_required(VERSION 3.25)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 11)

add_executable(ThreadPool main.cpp src/thread_pool.h src/thread_pool.cpp)
