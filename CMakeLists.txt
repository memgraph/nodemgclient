cmake_minimum_required(VERSION 3.10)

find_program(CLANG_FOUND clang)
find_program(CLANGXX_FOUND clang++)
if (CLANG_FOUND AND CLANGXX_FOUND)
  set(CMAKE_C_COMPILER ${CLANG_FOUND})
  set(CMAKE_CXX_COMPILER ${CLANGXX_FOUND})
else()
  message(FATAL_ERROR "Clang and Clang++ are required!")
endif()

project(nodemgclient VERSION 0.1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(C_STANDARD_REQUIRED ON)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_options INTERFACE)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Define the addon.
include_directories(${CMAKE_JS_INC})
set(SOURCE_FILES src/addon.cpp src/connection.cpp src/glue.cpp
                 src/record.cpp src/cursor.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE
                      ${CMAKE_JS_LIB} mgclient project_warnings project_options)
target_compile_options(${PROJECT_NAME} PRIVATE "-fno-exceptions")

# Include N-API wrappers.
execute_process(COMMAND node -p "require('node-addon-api').include"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${NODE_ADDON_API_DIR})

# Added because of BigInt support.
add_definitions(-DNAPI_EXPERIMENTAL)
# Cpp exceptions are disabled because the addon has to work with C primitives.
# Any throw increases a chance for a memory leak because there is no room for
# proper cleanup.
add_definitions(-DNAPI_DISABLE_CPP_EXCEPTIONS)
