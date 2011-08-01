cmake_minimum_required (VERSION 2.8)
project (j2dragline)
set (J2DRAGLINE_VERSION_MAJOR 4)
set (J2DRAGLINE_VERSION_MINOR 0)

enable_testing()
    
include_directories(
  ${PROJECT_BINARY_DIR}
  Util
  Modbus
  System
)

find_package(Boost REQUIRED)
set (Boost_LIBS boost_signals boost_chrono boost_system boost_thread-mt)
include_directories(${Boost_INCLUDE_DIR})
add_subdirectory (Config)
add_subdirectory (System)
add_subdirectory (Geometry)
add_subdirectory (Doc)

link_directories(${Boost_LIBRARY_DIRS})
add_executable (dsm main.cpp)
set (J2DRAGLINE_LIBS 
    System
    Geometry
    ${Boost_LIBS}
    CACHE J2Dragline Libraries)
target_link_libraries(dsm ${J2DRAGLINE_LIBS})

# Build and run unit tests
enable_testing()
find_package(GTest REQUIRED)
file(GLOB_RECURSE TEST_SRCS "Tests/*Test.cpp")
link_directories(${GTEST_LIBRARY_DIR})
add_executable(unittest-all ${TEST_SRCS})
include_directories(${GTEST_INCLUDE_DIR})
target_link_libraries(unittest-all ${J2DRAGLINE_LIBS} ${GTEST_BOTH_LIBRARIES})
file(COPY Tests/TestData DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
gtest_add_tests(unittest-all ${TEST_SRCS})

# Make sure the build is out of source
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
if(insource)
  file(REMOVE CMakeCache.txt)
  file(REMOVE CMakeFiles)
  message(FATAL_ERROR "Do not build in your source dir please :)")
endif(insource)

