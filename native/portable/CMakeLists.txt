cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(OpenGTPortable LANGUAGES CXX)
    include(CTest)
endif()

# Deliberately contains no renderer backend, capture I/O, DLL or host window.
add_library(opengt_geometry STATIC ../src/geometry_stitcher.cpp)
target_include_directories(opengt_geometry PUBLIC ../include)
target_compile_features(opengt_geometry PUBLIC cxx_std_17)

add_library(opengt_platform_io STATIC ../platform/common/stdio_files.cpp)
target_include_directories(opengt_platform_io PUBLIC ../include)
target_compile_features(opengt_platform_io PUBLIC cxx_std_17)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING)
    add_executable(portable_geometry_tests ../tests/geometry_stitcher_tests.cpp)
    target_link_libraries(portable_geometry_tests PRIVATE opengt_geometry)
    add_test(NAME portable_geometry COMMAND portable_geometry_tests)

    add_executable(platform_io_tests ../tests/platform_io_tests.cpp)
    target_link_libraries(platform_io_tests PRIVATE opengt_platform_io)
    add_test(NAME platform_io COMMAND platform_io_tests)
endif()
