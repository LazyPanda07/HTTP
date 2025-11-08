cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install-dir CACHE STRING "")
option(BUILD_SHARED_LIBS "" OFF)

if (UNIX)
	add_definitions(-D__LINUX__)
endif (UNIX)

if (${BUILD_SHARED_LIBS})
	add_definitions(-DHTTP_DLL)
endif ()

project(HTTP VERSION 1.14.0)

add_library(
	${PROJECT_NAME}
	src/HttpBuilder.cpp
	src/HttpParser.cpp
	src/HttpUtility.cpp
	src/HttpParseException.cpp
)

if (DEFINED ENV{MARCH} AND NOT "$ENV{MARCH}" STREQUAL "")
	target_compile_options(${PROJECT_NAME} PRIVATE -march=$ENV{MARCH})
endif()	

if (NOT TARGET JSON)
	add_subdirectory(JSON)
endif()

target_include_directories(
	${PROJECT_NAME} PUBLIC
	include
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	JSON
)

install(
	TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/dll
)

install(DIRECTORY include DESTINATION .)
