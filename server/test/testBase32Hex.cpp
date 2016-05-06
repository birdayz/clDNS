#include "gtest/gtest.h"
#include "Base32Hex.h"
#include <chrono>

TEST(TEST_BASE32HEX, test_tobase32) {
    NSEC3HashB32 base32hex = NSEC3HashB32fromCString("6NHNC1H8O864SH7IL5HPBAN57VJRHLD3");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    unsigned char expected[20] = {53, 227, 118, 6, 40, 194, 12, 78, 68, 242, 169, 99, 149, 170, 229, 63, 231, 184, 213,
                                  163};

    unsigned char result[20];
    int num_times = 1000;
    for (int i = 0; i < num_times; i++) {
        fromBase32Hex(base32hex, &result[0]);
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();


    std::cout << "Time difference ns= " <<
    (std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / num_times <<
    std::endl;

    EXPECT_TRUE(memcmp(expected, result, 20) == 0);
}

TEST(TEST_BASE32HEX, test_frombase32) {
    NSEC3HashB32 base32hex = NSEC3HashB32fromCString("6NHNC1H8O864SH7IL5HPBAN57VJRHLD3");

    unsigned char expected[20] = {53, 227, 118, 6, 40, 194, 12, 78, 68, 242, 169, 99, 149, 170, 229, 63, 231, 184, 213,
                                  163};
    unsigned char result[20];

    int len = fromBase32Hex(base32hex, &result[0]);

    EXPECT_TRUE(memcmp(expected, result, 20) == 0);
    EXPECT_EQ(20, len);

}
