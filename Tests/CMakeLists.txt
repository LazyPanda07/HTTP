cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(GTEST_VERSION 1.17.0)

project(Tests)

include(FetchContent)

FetchContent_Declare(
	gtest
	GIT_REPOSITORY https://github.com/google/googletest
	GIT_TAG v${GTEST_VERSION}
)

FetchContent_MakeAvailable(gtest)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

if (UNIX)
    set(DLL ${PROJECT_SOURCE_DIR}/../HTTP/lib/libHTTP.so)
	set(COPY_PATH lib)
	set(PATTERN_INSTALL *.so)
    add_definitions(-D__LINUX__)
else ()
    set(DLL ${PROJECT_SOURCE_DIR}/../HTTP/dll/HTTP.dll)
	set(COPY_PATH dll)
	set(PATTERN_INSTALL *.dll)
endif (UNIX)

if(EXISTS ${DLL})
    add_definitions(-DHTTP_DLL)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/../HTTP/${COPY_PATH}/ DESTINATION ${CMAKE_BINARY_DIR}/bin FILES_MATCHING PATTERN "${PATTERN_INSTALL}")
endif()

add_executable(
	${PROJECT_NAME}
	main.cpp
	src/HTTPParserTests.cpp
	src/HTTPBuilderTests.cpp
	src/HTTPTestUtils.cpp
	src/HTTPResponseTests.cpp
)

target_include_directories(
	${PROJECT_NAME} PRIVATE
	include
	${PROJECT_SOURCE_DIR}/../include
	${PROJECT_SOURCE_DIR}/../JSON/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/../HTTP/lib
)

target_link_libraries(
	${PROJECT_NAME}
	HTTP
	JSON
	gtest
	gtest_main
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)
