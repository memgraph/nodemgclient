cmake_minimum_required(VERSION 3.10)

project(nodemgclient)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_JS_INC})

set(SOURCE_FILES src/example.c)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} mgclient)

add_definitions(-DNAPI_VERSION=3)
