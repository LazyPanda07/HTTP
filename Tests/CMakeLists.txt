cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
set(CMAKE_CXX_STANDARD 20)

find_package(GTest)

if(NOT ${GTest_FOUND})
	set(GTEST_BOTH_LIBRARIES gtest gtest_main)
	set(GTEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/gtest/include/)
endif()

project(Tests)

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
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/include/
	${PROJECT_SOURCE_DIR}/../src/
	${PROJECT_SOURCE_DIR}/../JSON/src/
	${GTEST_INCLUDE_DIRS}
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/gtest/lib/
	${PROJECT_SOURCE_DIR}/../HTTP/lib/
)

target_link_libraries(
	${PROJECT_NAME}
	HTTP
	JSON
	${GTEST_BOTH_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)
