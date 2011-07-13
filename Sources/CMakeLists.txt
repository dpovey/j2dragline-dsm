cmake_minimum_required (VERSION 2.6)
project (j2dragline)
enable_testing()
set (J2DRAGLINE_VERSION_MAJOR 4)
set (J2DRAGLINE_VERSION_MINOR 0)
enable_testing()
set (GTEST_ROOT ${PROJECT_SOURCES_DIR}/gtest)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
set (BOOST_LIBS boost_signals)
    
include_directories(
  ${PROJECT_BINARY_DIR}
  Util
  Modbus
  Nexus
)
add_subdirectory (Nexus)
add_subdirectory (Config)
add_subdirectory (Doc)
add_subdirectory (Tests)
add_executable (dsm main.cpp)
target_link_libraries(Nexus)

# Make sure the build is out of source
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
if(insource)
  file(REMOVE CMakeCache.txt)
  file(REMOVE CMakeFiles)
  message(FATAL_ERROR "Do not build in your source dir please :)")
endif(insource)

