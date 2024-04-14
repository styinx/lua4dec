cmake_minimum_required(VERSION 3.21)

project(lua4dec)


#
# System information and configuration
#

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH "32")
endif()

if(NOT TARGET_ARCH)
    set(TARGET_ARCH ${ARCH})
elseif(NOT (TARGET_ARCH MATCHES "^(32|64)$"))
    message(FATAL_ERROR "TARGET_ARCH must be either 32 or 64")
endif()

set(CMAKE_DEBUG_POSTFIX _deb)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIG "deb")
else()
    set(CONFIG "rel")
endif()

set(BUILD_FOLDER ${CMAKE_SYSTEM_NAME}/${ARCH}/${CONFIG})


#
# Paths
#

# Local paths
set(DIR_ROOT "${PROJECT_SOURCE_DIR}")
set(DIR_INCLUDE "${DIR_ROOT}/include")
set(DIR_SOURCE "${DIR_ROOT}/source")
set(DIR_LIBRARY "${DIR_ROOT}/lib")
set(DIR_TEST "${DIR_ROOT}/test")

# Binary output paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DIR_LIBRARY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DIR_LIBRARY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DIR_ROOT}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${DIR_LIBRARY}/deb")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${DIR_LIBRARY}/deb")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${DIR_ROOT}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${DIR_LIBRARY}/rel")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${DIR_LIBRARY}/rel")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${DIR_ROOT}")


#
# Compiler settings
#

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_FLAGS "-Wall -Wextra -ansi -pedantic -Wno-format -std=c++17")
    set(COMPILER_FLAGS_DEBUG "-MTd")
    set(COMPILER_FLAGS_RELEASE "-O3 -DNDEBUG -MT")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_FLAGS "")
    set(COMPILER_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1 /MTd")
    set(COMPILER_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MT")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILER_FLAGS} ${COMPILER_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_FLAGS} ${COMPILER_FLAGS_RELEASE}")

# Macros
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    TARGET_ARCH=${TARGET_ARCH}
)

# Include paths
include_directories(
    ${DIR_INCLUDE}
    ${DIR_SOURCE}
)

# Linker paths
link_directories(
    ${DIR_LIBRARY}
)

#
# Dependencies
#

add_subdirectory(lua4)


#
# Sources
#

set(SOURCES_LIB
    source/errors.cpp
    source/lua4dec.cpp
    source/ast/ast.cpp
    source/lua/lua.cpp
    source/parser/parser.cpp
)

set(SOURCES_EXE
    source/main.cpp
)

# Source filters
source_group("source"         FILES source/lua4dec.cpp source/lua4dec.hpp
                                    source/errors.cpp source/errors.hpp)
source_group("source/ast"     FILES source/ast/ast.cpp source/ast/ast.hpp)
source_group("source/lua"     FILES source/lua/lua.cpp source/lua/lua.hpp)
source_group("source/parser"  FILES source/parser/parser.cpp source/parser/parser.hpp)


#
# Binaries
#

set(EXE luadec_${TARGET_ARCH})
set(LIB lua4dec_${TARGET_ARCH})

add_library(${LIB} ${SOURCES_LIB})
set_target_properties(${LIB} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_property(TARGET ${LIB} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(${EXE} ${SOURCES_EXE})
target_link_libraries(${EXE} ${LIB})
set_target_properties(${EXE} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_property(TARGET ${EXE} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(test tests/test.cpp)
target_link_libraries(test ${LIB})
set_property(TARGET test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(differ tests/differ.cpp)
target_link_libraries(differ ${LIB})
set_property(TARGET differ PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
