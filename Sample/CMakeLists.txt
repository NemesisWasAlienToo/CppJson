cmake_minimum_required(VERSION 3.12)

project(IncTest VERSION 1.0.0 LANGUAGES CXX)

find_package(CoreFormatJson REQUIRED)

add_executable(${PROJECT_NAME} Main.cpp)

target_link_libraries(${PROJECT_NAME} Core::Format::Json)
