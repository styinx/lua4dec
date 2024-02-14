cmake_minimum_required(VERSION 3.21)

project(lua4dec)


#
# System information and configuration
#

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH "x32")
endif()

if(NOT TARGET_ARCH)
    set(TARGET_ARCH ${ARCH})
endif()

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
    source/lua4dec.cpp
    source/lua4dec.hpp
    source/ast/ast.cpp
    source/ast/ast.hpp
    source/lua/lua.cpp
    source/lua/lua.hpp
    source/parser/parser.cpp
    source/parser/parser.hpp
)

set(SOURCES_EXE
    source/main.cpp
)

# Source filters
source_group("source"         FILES source/lua4dec.cpp source/lua4dec.hpp)
source_group("source/ast"     FILES source/ast/ast.cpp source/ast/ast.hpp)
source_group("source/lua"     FILES source/lua/lua.cpp source/lua/lua.hpp)
source_group("source/parser"  FILES source/parser/parser.cpp source/parser/parser.hpp)


#
# Binaries
#

set(EXE luadec_${TARGET_ARCH}_${MODE})
set(LIB lua4dec_${TARGET_ARCH}_${MODE})

add_library(${LIB} ${SOURCES_LIB})

add_executable(${EXE} ${SOURCES_EXE})
target_link_libraries(${EXE} ${LIB})

add_executable(test tests/test.cpp)
target_link_libraries(test ${LIB})
