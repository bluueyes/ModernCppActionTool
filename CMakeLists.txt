cmake_minimum_required(VERSION 3.12)
project(reaction VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB EXAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp
)
foreach(example_source ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_source} NAME_WE)
    add_executable(${example_name} ${example_source})
    target_link_libraries(${example_name} PRIVATE ${PROJECT_NAME})
    
endforeach()



find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

if(GTEST_FOUND)
    enable_testing()
    file(GLOB TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
    )
    add_executable(run_test ${TEST_SOURCES})
    target_link_libraries(run_test PRIVATE ${PROJECT_NAME} gtest gtest_main Threads::Threads)
    add_test(NAME reactionTest COMMAND run_test)
    message(STATUS "GTest found, tests will be built.")
else()
    message(STATUS "GTest not found, skipping tests.")

endif()
