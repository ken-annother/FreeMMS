cmake_minimum_required(VERSION 3.10.1)
project(freemms VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)

macro(FMLOG MSG)
    string(APPEND FM_BUILD_STATS "${MSG}: ${${MSG}}\n")
endmacro()

include(cmake/components/freemms-core.cmake)
include(cmake/components/freemms-cli.cmake)

set(SPDLOG_BUILD_SHARED ON)
add_subdirectory(3rdparty/spdlog)

option(BUILD_TESTING "Build Tests" OFF)
FMLOG(BUILD_TESTING)

set(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem program_options)
FIND_PACKAGE(Iconv REQUIRED)


add_library(freemms_core STATIC
        ${FREEMMS_CORE_SOURCES})

target_include_directories(freemms_core PRIVATE
        ${Boost_INCLUDE_DIRS})

target_include_directories(freemms_core PUBLIC
        ${FREEMMS_BASEDIR_CORE_HEADERS})

target_link_libraries(freemms_core PUBLIC
        spdlog::spdlog
        Iconv::Iconv
        Boost::filesystem
        Boost::program_options)

add_executable(mms2plain ${FREEMMS_CLI_MMS2PLAIN_SOURCES})
target_link_libraries(mms2plain freemms_core)

add_executable(plain2mms ${FREEMMS_CLI_PLAIN2MMS_SOURCES})
target_link_libraries(plain2mms freemms_core)


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
