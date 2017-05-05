cmake_minimum_required(VERSION 3.6)
project(http_parser)

set(CMAKE_CXX_STANDARD 11)

# Boost
find_package(Boost 1.57.0 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Catch
include_directories(vendor)

include_directories(include)
set(SOURCE_FILES test/request_parser.test.cpp)
add_executable(runTests ${SOURCE_FILES})

target_link_libraries(runTests ${Boost_LIBRARIES})