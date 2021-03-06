cmake_minimum_required(VERSION 3.5)
project(shadows CXX C)

add_compile_options(-pipe)
add_compile_options(-march=native -Ofast -fomit-frame-pointer -ftree-vectorize)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(coroutine HAS_COROUTINE)
if (NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    # Do nothing
elseif (HAS_ANY AND HAS_STRING_VIEW AND HAS_COROUTINE)
    set(CMAKE_CXX_STANDARD 20)
elseif (HAS_ANY AND HAS_STRING_VIEW)
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
        main.cpp

        plugins/database.hpp
        plugins/database.cpp

        sqlite/sqlite3.h
        sqlite/sqlite3.c

        plugins/graph.hpp
        plugins/graph.cpp
        plugins/grid.hpp
        plugins/grid.cpp
        plugins/solar.hpp
        plugins/solar.cpp
        plugins/path.cpp
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        -lsqlite3
        -DSQLITE_ENABLE_MATH_FUNCTIONS

        -DSQLITE_DQS=0
        -DSQLITE_DEFAULT_MEMSTATUS=0
        -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=1
        -DSQLITE_LIKE_DOESNT_MATCH_BLOBS
        -DSQLITE_MAX_EXPR_DEPTH=0
        -DSQLITE_OMIT_DECLTYPE
        -DSQLITE_OMIT_DEPRECATED
        -DSQLITE_OMIT_PROGRESS_CALLBACK
        -DSQLITE_OMIT_SHARED_CACHE
        -DSQLITE_USE_ALLOCA
        # -DSQLITE_OMIT_AUTOINIT
        )

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon
add_subdirectory(drogon)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
#find_package(Drogon CONFIG REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

# ##############################################################################

target_compile_options(drogon PRIVATE -DCMAKE_BUILD_TYPE=Release)

if (CMAKE_CXX_STANDARD LESS 17)
    # With C++14, use boost to support any, string_view and filesystem
    message(STATUS "use c++14")
    find_package(Boost 1.61.0 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)
elseif (CMAKE_CXX_STANDARD LESS 20)
    message(STATUS "use c++17")
else ()
    message(STATUS "use c++20")
endif ()

#target_compile_options(${PROJECT_NAME}  PRIVATE -lsqlite3 -DSQLITE_ENABLE_MATH_FUNCTIONS)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lsqlite3 -DSQLITE_ENABLE_MATH_FUNCTIONS")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lsqlite3 -DSQLITE_ENABLE_MATH_FUNCTIONS")

aux_source_directory(controllers CTL_SRC)
aux_source_directory(filters FILTER_SRC)
aux_source_directory(plugins PLUGIN_SRC)
aux_source_directory(models MODEL_SRC)

drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
        ${CMAKE_CURRENT_BINARY_DIR})
# use the following line to create views with namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${SRC_DIR}
        ${CTL_SRC}
        ${FILTER_SRC}
        ${PLUGIN_SRC}
        ${MODEL_SRC})

# ##############################################################################
# uncomment the following line for dynamically loading views 
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

#add_subdirectory(test)
