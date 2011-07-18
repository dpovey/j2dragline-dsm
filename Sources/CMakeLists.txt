cmake_minimum_required (VERSION 2.8)
project (j2dragline)
enable_testing()
set (J2DRAGLINE_VERSION_MAJOR 4)
set (J2DRAGLINE_VERSION_MINOR 0)

enable_testing()
    
include_directories(
  ${PROJECT_BINARY_DIR}
  Util
  Modbus
  EventRouter
)

find_package(BOOST REQUIRED)
set (BOOST_LIBS boost_signals)
add_subdirectory (Config)
add_subdirectory (EventRouter)
add_subdirectory (Doc)
add_subdirectory (Tests)
link_directories(${Boost_LIBRARY_DIRS})
add_executable (dsm main.cpp)
set (J2DRAGLINE_LIBS 
	EventRouter 
	${BOOST_LIBS} 
	CACHE J2Dragline Libraries)
target_link_libraries(dsm ${J2DRAGLINE_LIBS})
include_directories(${Boost_INCLUDE_DIR})

# Make sure the build is out of source
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
if(insource)
  file(REMOVE CMakeCache.txt)
  file(REMOVE CMakeFiles)
  message(FATAL_ERROR "Do not build in your source dir please :)")
endif(insource)

