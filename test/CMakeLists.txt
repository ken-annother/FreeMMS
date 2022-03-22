include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

message("UUU->" ${CMAKE_CURRENT_SOURCE_DIR})

function(ADD_FM_TEST TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(
            ${TEST_NAME}
            PRIVATE
            gtest_main
            $<$<TARGET_EXISTS:freemms_core>:freemms_core>
    )
    gtest_discover_tests(${TEST_NAME})
endfunction()

file(GLOB TEST_RESOURCE resource/*)
file(COPY ${TEST_RESOURCE} DESTINATION resource)

file(GLOB METADATA ${CMAKE_SOURCE_DIR}/src/core/metadata/*)
file(COPY ${METADATA} DESTINATION metadata)

ADD_FM_TEST(hello_test src/hello_test.cpp)