cmake_minimum_required(VERSION 3.10.1)
project(freemms VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)

macro(FMLOG MSG)
    string(APPEND FM_BUILD_STATS "${MSG}: ${${MSG}}\n")
endmacro()

include(cmake/components/freemms-core.cmake)

set(SPDLOG_BUILD_SHARED ON)
add_subdirectory(3rdparty/spdlog)

option(BUILD_TESTING "Build Tests" OFF)
FMLOG(BUILD_TESTING)

FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem)
FIND_PACKAGE(Iconv REQUIRED)

add_library(freemms_core SHARED
        ${FREEMMS_CORE_SOURCES})

target_include_directories(freemms_core PUBLIC
        ${FREEMMS_BASEDIR_CORE_HEADERS})
target_link_libraries(freemms_core PRIVATE spdlog::spdlog Iconv::Iconv boost_filesystem boost_system)

if (BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif ()


message("")
message("======== FreeMMS Build Statistics ========")
message("${FM_BUILD_STATS}")
message("=========================================")
message("")
