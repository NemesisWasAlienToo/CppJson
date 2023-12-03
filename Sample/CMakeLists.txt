cmake_minimum_required(VERSION 3.12)

project(CppJson VERSION 1.0.0 LANGUAGES CXX)

option(UseInstalled "Uses the installed veriosn of the library or the in-source one" OFF)

add_executable(${PROJECT_NAME} Json.cpp)

if(UseInstalled)
    find_package(Core.Format.Json REQUIRED)
    target_link_libraries(${PROJECT_NAME} Core::Format::Json)
    message("Using the installed version of the library")
else()
    find_package(ctre REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ../Library)
    target_link_libraries(${PROJECT_NAME} ctre::ctre)
    message("Using the in-source version of the library")
endif()
