cmake_minimum_required(VERSION 3.27.0)

project(Tests)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
set(CMAKE_CXX_STANDARD 20)

if (UNIX)
    set(DLL ${PROJECT_SOURCE_DIR}/../HTTP/libHTTP.so)
	set(PATTERN_INSTALL *.so)
    add_definitions(-D__LINUX__)
else ()
    set(DLL ${PROJECT_SOURCE_DIR}/../HTTP/HTTP.dll)
	set(PATTERN_INSTALL *.dll)
endif (UNIX)

if(EXISTS ${DLL})
    add_definitions(-DHTTP_DLL)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/../HTTP/ DESTINATION bin FILES_MATCHING PATTERN "${PATTERN_INSTALL}")
endif()

add_executable(
	${PROJECT_NAME}
	main.cpp
	src/HTTPParserTests.cpp
	src/HTTPBuilderTests.cpp
	src/HTTPTestUtils.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/include/
	${PROJECT_SOURCE_DIR}/gtest/include/
	${PROJECT_SOURCE_DIR}/../src/
	${PROJECT_SOURCE_DIR}/../JSON/src/
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/gtest/lib/
	${PROJECT_SOURCE_DIR}/../HTTP/
)

target_link_libraries(
	${PROJECT_NAME}
	gtest
	gtest_main
	HTTP
	JSON
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY data DESTINATION bin)
