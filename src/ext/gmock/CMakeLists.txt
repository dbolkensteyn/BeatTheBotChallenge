cmake_minimum_required(VERSION 2.8)
project(gmock_builder)
include(ExternalProject)

ExternalProject_Add(googlemock
    URL ${PROJECT_SOURCE_DIR}/gmock-1.7.0.zip
    CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
               -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
               -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
               -Dgtest_force_shared_crt=ON
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    # Disable install step
    INSTALL_COMMAND ""
)

# Specify include dir
ExternalProject_Get_Property(googlemock source_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/gtest/include ${source_dir}/include PARENT_SCOPE)

# Specify MainTest's link libraries
ExternalProject_Get_Property(googlemock binary_dir)
set(GTEST_LIBS_DIR ${binary_dir}/gtest PARENT_SCOPE)
