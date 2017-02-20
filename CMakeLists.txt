cmake_minimum_required (VERSION 2.8)

set(PROJECT Client)
project (${PROJECT})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
# 	add_definitions(-g)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

aux_source_directory(sources SOURCES)

set(SOURCES ${SOURCES}
	main.cpp
)

include_directories(
	includes/GUI
	includes/Client
)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(${PROJECT} ${SOURCES})

target_link_libraries(${PROJECT}
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
)
