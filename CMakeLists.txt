cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 17)

if (UNIX)
	add_definitions(-D__LINUX__)
endif (UNIX)

if (${BUILD_SHARED_LIBS} STREQUAL "ON")
	add_definitions(-DHTTP_DLL)
endif ()

project(HTTP)

add_library(
	${PROJECT_NAME}
	src/HTTPBuilder.cpp
	src/HTTPParser.cpp
	src/HTTPUtility.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	JSON/src
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	JSON
)

add_subdirectory(JSON)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	install(DIRECTORY src/ DESTINATION ${CMAKE_BINARY_DIR}/bin/include FILES_MATCHING PATTERN "*.h")
endif ()
