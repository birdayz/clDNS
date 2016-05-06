#include "gtest/gtest.h"
#include "testSupport.h"

char *kernelDir = NULL;
using namespace testing;

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    kernelDir = argv[1];
    return RUN_ALL_TESTS();
}
