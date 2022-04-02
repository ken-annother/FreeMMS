#include <gtest/gtest.h>
#include "MMSEngine.h"

using namespace std;

int test() {
    auto engine = new MMSEngine();
    string mmsHexFilePath = "resource/160767603214113640";
    string result = engine->convert2Plain(mmsHexFilePath);
    printf("result is:  \n%s\n", result.c_str());

    engine->convert2PlainFile(mmsHexFilePath, "160767603214113640.txt", true);
    engine->convert2PlainDirectory(mmsHexFilePath, "160767603214113640");
    delete engine;
    return 1;
}


int test2() {
    auto engine = new MMSEngine();
    string mmsPlainPath = "resource/160767603214113640.txt";
    string mmsHexOutputPath = "160767603214113640_new.mms";
    engine->convert2mmsHex(mmsPlainPath, mmsHexOutputPath);
    delete engine;
    return 1;
}

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    int result = test();
    EXPECT_EQ(result, 1);
}

//TEST(HelloTest2, BasicAssertions2) {
//    int result = test2();
//    EXPECT_EQ(result, 1);
//}