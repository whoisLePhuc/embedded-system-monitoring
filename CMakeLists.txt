cmake_minimum_required(VERSION 3.10)
project(EmbeddedSystemMonitoring VERSION 1.0)

file(GLOB_RECURSE SRC_FILES "src/*.c")

include_directories(
    inc/
    inc/cpuManager
    inc/memoryManager
    inc/networkManager
    inc/storageManager
    inc/systemManager
)

add_executable(systemMonitoring main.c ${SRC_FILES})

