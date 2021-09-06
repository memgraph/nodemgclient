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

include(ExternalProject)

# Add mgclient as a library.
ExternalProject_Add(mgclient-proj
                    PREFIX         mgclient
                    GIT_REPOSITORY https://github.com/memgraph/mgclient.git
                    GIT_TAG        4940963e0d2ebfdc519f2120843bf403d74c3f5e
                    CMAKE_ARGS     "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
                                   "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                                   "-DBUILD_CPP_BINDINGS=ON"
                    INSTALL_DIR    "${PROJECT_BINARY_DIR}/mgclient")
ExternalProject_Get_Property(mgclient-proj install_dir)
set(MGCLIENT_ROOT ${install_dir})
set(MGCLIENT_INCLUDE_DIRS ${MGCLIENT_ROOT}/include
    CACHE INTERNAL "Path to mgclient include directory")
set(MGCLIENT_LIBRARY_PATH ${MGCLIENT_ROOT}/lib/libmgclient.so)
set(MGCLIENT_LIBRARY mgclient-lib)
add_library(${MGCLIENT_LIBRARY} SHARED IMPORTED)
set_target_properties(${MGCLIENT_LIBRARY} PROPERTIES
                      IMPORTED_LOCATION ${MGCLIENT_LIBRARY_PATH})
add_dependencies(${MGCLIENT_LIBRARY} mgclient-proj)

# Define the addon.
include_directories(${CMAKE_JS_INC})
set(SOURCE_FILES src/addon.cpp
                 src/client.cpp
                 src/glue.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE
                      ${CMAKE_JS_LIB} ${MGCLIENT_LIBRARY}
                      project_warnings project_options)
target_include_directories(${PROJECT_NAME} PRIVATE ${MGCLIENT_INCLUDE_DIRS})
# TODO(gitbuda): Make a decisions about CPP exceptions (mgclinet cpp throws).
#                Document why enabled, how to handle and why we return optional.
# If at any time we decide to disable exceptions with the combination of
# NAPI_THROW + std::nullopt in case of an error, the code should work as is.
# On promise ok, where there is not value, return null, otherwise set error
target_compile_options(${PROJECT_NAME} PRIVATE "")
add_definitions(-DNAPI_CPP_EXCEPTIONS)

# Include N-API wrappers.
execute_process(COMMAND node -p "require('node-addon-api').include"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${NODE_ADDON_API_DIR})
