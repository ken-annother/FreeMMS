#include <gtest/gtest.h>
#include "MMSEngine.h"

using namespace std;

int test() {
    auto engine = new MMSEngine();
    string mmsHexFilePath = "resource/160767603214113640";
    string result = engine->convert2Plain(mmsHexFilePath);
    printf("result is:  \n%s\n", result.c_str());
    return 1;
}


// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    int result = test();
    EXPECT_EQ(result, 1);
}