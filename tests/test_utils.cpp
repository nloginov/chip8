#include <gtest/gtest.h>
#include <utils/bitutils.h>

TEST(TestBitutils, TestGetOctetAt) {
    ASSERT_EQ(0xD, GetOctetAt<1>(0xABCD));
    ASSERT_EQ(0xC, GetOctetAt<2>(0xABCD));
    ASSERT_EQ(0xB, GetOctetAt<3>(0xABCD));
    ASSERT_EQ(0xA, GetOctetAt<4>(0xABCD));
}

TEST(TestBitutils, TestLastGetOctet) {
    ASSERT_EQ(0xA, GetLastOctet(0xABCD));
    ASSERT_EQ(0x0, GetLastOctet(0x0001));
}


TEST(TestBitutils, TestGetOctetsRange) {
    ASSERT_EQ(0xBC, (GetOctetsRange<2,3>(0xABCD)));
    ASSERT_EQ(0xBCD, (GetOctetsRange<1,3>(0xABCD)));
    ASSERT_EQ(0xABC, (GetOctetsRange<2,4>(0xABCD)));
    ASSERT_EQ(0xA, (GetOctetsRange<4,4>(0xABCD)));

    ASSERT_EQ(sizeof(uint8_t), sizeof(GetOctetsRange<2,3>(0xABCD)));
    ASSERT_EQ(sizeof(uint8_t), sizeof(GetOctetsRange<3,3>(0xABCD)));
    ASSERT_EQ(sizeof(uint16_t), sizeof(GetOctetsRange<1,3>(0xABCD)));
    ASSERT_EQ(sizeof(uint16_t), sizeof(GetOctetsRange<1,4>(0xABCD)));
}
