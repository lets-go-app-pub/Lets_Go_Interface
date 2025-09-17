cmake_minimum_required(VERSION 3.20)
project(Lets_Go_Interface)

set(CMAKE_CXX_STANDARD 20)

# Append the local grpc version to be found using find_packages()
list(APPEND CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH_LETS_GO})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

set(CLIENT_SERVER_PROTO_FILE_DIRECTORY $ENV{SERVER_SPECIFIC_PROTO_FILE_DIRECTORY_LETS_GO})
set(SERVER_SPECIFIC_PROTO_FILE_DIRECTORY $ENV{CLIENT_SERVER_PROTO_FILE_DIRECTORY_LETS_GO})

# Used to include proto_client_server_shared inside of proto_server_specific .proto files
set(SERVER_SPECIFIC_PROTO_INCLUDE_PATHS "/home/jeremiah/CLionProjects/Lets_Go_Interface/cmake-build-debug/generated_proto_client_server_shared")

add_subdirectory(project)
add_subdirectory(${CLIENT_SERVER_PROTO_FILE_DIRECTORY} generated_proto_client_server_shared)
add_subdirectory(${SERVER_SPECIFIC_PROTO_FILE_DIRECTORY} generated_proto_server_specific)

#find_package(Qt6 COMPONENTS
#        Core
#        Gui
#        Widgets
#        REQUIRED)
#
#add_executable(Lets_Go_Interface main.cpp)
#target_link_libraries(Lets_Go_Interface
#        Qt::Core
#        Qt::Gui
#        Qt::Widgets
#        )



