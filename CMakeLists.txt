cmake_minimum_required(VERSION 3.15)
project(RemoteConsole)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(RemoteConsole ${SOURCES})

target_link_libraries(RemoteConsole ws2_32 advapi32)

target_include_directories(RemoteConsole PRIVATE src)

set_target_properties(RemoteConsole PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
