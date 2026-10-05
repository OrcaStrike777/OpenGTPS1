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

add_library(opengt_guest STATIC ../runtime/guest.cpp ../runtime/ps1_interrupts.cpp ../runtime/ps1_dma.cpp ../runtime/ps1_gpu.cpp ../runtime/ps1_cdrom.cpp ../runtime/ps1_memcontrol.cpp ../runtime/startup_services.cpp)
target_include_directories(opengt_guest PUBLIC ../include)
target_compile_features(opengt_guest PUBLIC cxx_std_17)

add_library(opengt_guest_tests STATIC ../tests/recompiled/mips_suite.cpp ../tests/recompiled/boot_runtime_suite.cpp ../tests/interrupt_tests.cpp ../tests/bios_tests.cpp ../tests/dma_tests.cpp ../tests/gpu_tests.cpp ../tests/cdrom_tests.cpp ../tests/memcontrol_tests.cpp)
target_link_libraries(opengt_guest_tests PUBLIC opengt_guest)
option(OPENGT_GT2_PROBE "Build the locally generated, disc-validated GT2 hash probe" OFF)
option(OPENGT_GT2_BOOT "Build the locally generated bounded GT2 startup probe" OFF)
if(OPENGT_GT2_BOOT)
    target_sources(opengt_guest_tests PRIVATE ../../generated/old3ds/gt2_boot.cpp)
else()
    target_sources(opengt_guest_tests PRIVATE ../tests/boot_probe_stub.cpp)
endif()
if(OPENGT_GT2_PROBE OR OPENGT_GT2_BOOT)
    target_sources(opengt_guest_tests PRIVATE ../../generated/old3ds/gt2_probe.cpp)
else()
    target_sources(opengt_guest_tests PRIVATE ../tests/gt2_probe_stub.cpp)
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND BUILD_TESTING)
    add_executable(portable_geometry_tests ../tests/geometry_stitcher_tests.cpp)
    target_link_libraries(portable_geometry_tests PRIVATE opengt_geometry)
    add_test(NAME portable_geometry COMMAND portable_geometry_tests)

    add_executable(platform_io_tests ../tests/platform_io_tests.cpp)
    target_link_libraries(platform_io_tests PRIVATE opengt_platform_io)
    add_test(NAME platform_io COMMAND platform_io_tests)

    add_executable(guest_tests ../tests/guest_tests_main.cpp)
    target_link_libraries(guest_tests PRIVATE opengt_guest_tests)
    add_test(NAME native_mips COMMAND guest_tests)
endif()
