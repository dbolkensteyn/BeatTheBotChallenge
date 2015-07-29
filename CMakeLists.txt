cmake_minimum_required(VERSION 3.0)

# Main
project(BeatTheBotChallenge VERSION 0.1)
find_package(OpenCV REQUIRED)
add_executable(${PROJECT_NAME} src/main/robot.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Test
add_subdirectory(src/ext/gmock)
enable_testing()
set(TEST_PROJECT_NAME ${PROJECT_NAME}Tests)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${TEST_PROJECT_NAME} src/test/test.cpp)
add_dependencies(${TEST_PROJECT_NAME} googlemock)

if(NOT WIN32)
    target_link_libraries(${TEST_PROJECT_NAME}
        ${GTEST_LIBS_DIR}/libgtest.a
        ${GTEST_LIBS_DIR}/libgtest_main.a
        )
else()
    target_link_libraries(${TEST_PROJECT_NAME}
        debug ${GTEST_LIBS_DIR}/DebugLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_FIND_LIBRARY_SUFFIXES}
        optimized ${GTEST_LIBS_DIR}/ReleaseLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_FIND_LIBRARY_SUFFIXES}
        )
    target_link_libraries(${TEST_PROJECT_NAME}
        debug ${GTEST_LIBS_DIR}/DebugLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_FIND_LIBRARY_SUFFIXES}
        optimized ${GTEST_LIBS_DIR}/ReleaseLibs/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_FIND_LIBRARY_SUFFIXES}
        )
endif()

add_test(${TEST_PROJECT_NAME} ${TEST_PROJECT_NAME})
