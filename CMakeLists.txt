cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(nypkg)

set(CMAKE_CXX_STANDARD 17)

# JSON library
find_package(nlohmann_json REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(nypkg ${SOURCES})
target_include_directories(nypkg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(nypkg nlohmann_json::nlohmann_json)

