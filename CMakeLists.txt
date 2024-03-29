cmake_minimum_required(VERSION 3.10)

#### NOTES:
# * HOW TO CHANGE THE COMPILER?
#   If you want to use specific compiler, consider using -C | --cc | --cxx cmake-js flags.
#
# * WINDOWS AND MINGW ISSUES?
#   cmake-js doesn't support MinGW Makefiles -> https://github.com/cmake-js/cmake-js/issues/195
#   /DELAYLOAD:NODE.EXE: No such file or directory -> https://github.com/cmake-js/cmake-js/issues/200
#   cmake-js adds /DELAYLOAD:NODE.EXE + there is not an easy solution.
#   I've played with paths + setting the CMAKE_SHARED_LINKER_FLAGS but all that is N/A.
#   The following does not work because CMake changes the string to be Windows path.
#   set(CMAKE_SHARED_LINKER_FLAGS "/DELAYLOAD:/c/Program\\ Files/nodejs/node.exe")

project(nodemgclient VERSION 0.1.3)

if (WIN32 AND MINGW)
    message(FATAL_ERROR "ON WINDOWS BUILD UNDER MINGW NOT YET POSSIBLE")
endif()

include(ExternalProject)
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

set(MGCLIENT_GIT_TAG "v1.4.1" CACHE STRING "mgclient git tag")
set(MGCLIENT_LIBRARY      mgclient-lib)
set(MGCLIENT_INSTALL_DIR  ${CMAKE_BINARY_DIR}/mgclient)
set(MGCLIENT_INCLUDE_DIRS ${MGCLIENT_INSTALL_DIR}/include)
if (UNIX AND NOT APPLE)
  set(MGCLIENT_LIBRARY_PATH ${MGCLIENT_INSTALL_DIR}/lib/libmgclient.so)
elseif (WIN32)
  set(MGCLIENT_LIBRARY_PATH ${MGCLIENT_INSTALL_DIR}/lib/mgclient.dll)
elseif (APPLE)
  set(MGCLIENT_LIBRARY_PATH ${MGCLIENT_INSTALL_DIR}/lib/libmgclient.dylib)
endif()
ExternalProject_Add(mgclient-proj
  PREFIX           mgclient-proj
  GIT_REPOSITORY   https://github.com/memgraph/mgclient.git
  GIT_TAG          "${MGCLIENT_GIT_TAG}"
  CMAKE_ARGS       "-DCMAKE_INSTALL_PREFIX=${MGCLIENT_INSTALL_DIR}"
                   "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                   "-DBUILD_CPP_BINDINGS=ON"
                   "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
                   "-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}"
  BUILD_BYPRODUCTS "${MGCLIENT_LIBRARY_PATH}"
  INSTALL_DIR      "${PROJECT_BINARY_DIR}/mgclient"
)
add_library(${MGCLIENT_LIBRARY} SHARED IMPORTED)
target_compile_definitions(${MGCLIENT_LIBRARY} INTERFACE mgclient_shared_EXPORTS)
set_property(TARGET ${MGCLIENT_LIBRARY} PROPERTY IMPORTED_LOCATION ${MGCLIENT_LIBRARY_PATH})
if (WIN32)
  set_property(TARGET ${MGCLIENT_LIBRARY} PROPERTY IMPORTED_IMPLIB ${MGCLIENT_INSTALL_DIR}/lib/mgclient.lib)
endif()
add_dependencies(${MGCLIENT_LIBRARY} mgclient-proj)

# Define the addon.
include_directories(${CMAKE_JS_INC})
set(SOURCE_FILES src/addon.cpp src/client.cpp src/glue.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_compile_definitions(${PROJECT_NAME} PRIVATE -Dmgclient_shared_EXPORTS)
add_dependencies(${PROJECT_NAME} ${MGCLIENT_LIBRARY})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} ${MGCLIENT_LIBRARY} project_warnings project_options)
if (WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${MGCLIENT_INCLUDE_DIRS})
# C++ mgclient throws, which means this project has to enable exceptions. If at
# any time we decide to disable exceptions with the combination of NAPI_THROW +
# std::nullopt in case of an error, the code should work as is.
add_definitions(-DNAPI_CPP_EXCEPTIONS)
# Include N-API wrappers.
execute_process(COMMAND node -p "require('node-addon-api').include"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${NODE_ADDON_API_DIR}")
