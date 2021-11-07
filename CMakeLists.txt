cmake_minimum_required(VERSION 3.10)

# NOTES
#
# HOW TO CHANGE THE COMPILER?
# If you want to use specific compiler, consider using -C | --cc | --cxx cmake-js flags.
#
# WINDOWS AND MINGW ISSUES?
# cmake-js doesn't support MinGW Makefiles -> https://github.com/cmake-js/cmake-js/issues/195
# /DELAYLOAD:NODE.EXE: No such file or directory -> https://github.com/cmake-js/cmake-js/issues/200
# cmake-js adds /DELAYLOAD:NODE.EXE + there is not an easy solution -> Move to node-gyp
# I've played with paths + setting the CMAKE_SHARED_LINKER_FLAGS but all that is N/A.
# The following does not work because CMake changes the string to be Windows path.
# set(CMAKE_SHARED_LINKER_FLAGS "/DELAYLOAD:/c/Program\\ Files/nodejs/node.exe")

project(nodemgclient VERSION 0.1.0)

find_package(OpenSSL REQUIRED)

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

if (WIN32 AND MINGW)
    message(FATAL_ERROR "ON WINDOWS BUILD UNDER MINGW NOT YES POSSIBLE")
elseif (APPLE)
    message(FATAL_ERROR "ON APPLE BUILD NOT YET POSSIBLE")
endif()

# Add mgclient as an imported library.
ExternalProject_Add(mgclient-proj
                    PREFIX         mgclient
                    GIT_REPOSITORY https://github.com/memgraph/mgclient.git
                    GIT_TAG        63ffb016f2338e56ba7c5171d5f4938b6cec50f6
                    CMAKE_ARGS     "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
                                   "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                    INSTALL_DIR    "${PROJECT_BINARY_DIR}/mgclient")
ExternalProject_Get_Property(mgclient-proj install_dir)
set(MGCLIENT_ROOT ${install_dir})
set(MGCLIENT_INCLUDE_DIRS ${MGCLIENT_ROOT}/include
    CACHE INTERNAL "Path to mgclient include directory")
if (UNIX AND NOT APPLE)
    set(MGCLIENT_LIBRARY_PATH ${MGCLIENT_ROOT}/lib/libmgclient.so)
elseif (WIN32 AND MINGW)
    set(MGCLIENT_LIBRARY_PATH ${MGCLIENT_ROOT}/lib/libmgclient.dll)
endif()
message(STATUS "MGCLIENT_LIBRARY_PATH: ${MGCLIENT_LIBRARY_PATH}")
set(MGCLIENT_LIBRARY mgclient-lib)
add_library(${MGCLIENT_LIBRARY} SHARED IMPORTED)
set_property(TARGET ${MGCLIENT_LIBRARY} PROPERTY IMPORTED_LOCATION ${MGCLIENT_LIBRARY_PATH})
if (WIN32 AND MINGW)
    set_property(TARGET ${MGCLIENT_LIBRARY} PROPERTY IMPORTED_IMPLIB ${MGCLIENT_ROOT}/lib/libmgclient.dll.a)
endif()
add_dependencies(${MGCLIENT_LIBRARY} mgclient-proj)
message(STATUS "MGCLIENT_LIBRARY: ${MGCLIENT_LIBRARY}")

# Define the addon.
include_directories(${CMAKE_JS_INC})
set(SOURCE_FILES src/addon.cpp src/connection.cpp src/glue.cpp
                 src/record.cpp src/cursor.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
add_dependencies(${PROJECT_NAME} ${MGCLIENT_LIBRARY})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE
                      ${CMAKE_JS_LIB} ${MGCLIENT_LIBRARY}
                      project_warnings project_options)
# TODO(gitbuda): hpp wrappers don't work yet because of exceptions.
# Exceptions are disabled in this project. Decide and adjust.
target_include_directories(${PROJECT_NAME} PRIVATE ${MGCLIENT_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE "-fno-exceptions")

# Include N-API wrappers.
execute_process(COMMAND node -p "require('node-addon-api').include"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${NODE_ADDON_API_DIR}")

# Added because of BigInt support.
add_definitions(-DNAPI_EXPERIMENTAL)
# Cpp exceptions are disabled because the addon has to work with C primitives.
# Any throw increases a chance for a memory leak because there is no room for
# proper cleanup.
add_definitions(-DNAPI_DISABLE_CPP_EXCEPTIONS)
