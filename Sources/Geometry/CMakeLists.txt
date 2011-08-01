FILE(GLOB CPP_FILES "*.cpp")
FILE(GLOB HEADER_FILES "*.h")
add_library (Geometry ${CPP_FILES} ${HEADER_FILES})
