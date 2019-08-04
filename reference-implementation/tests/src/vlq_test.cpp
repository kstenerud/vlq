#include <gtest/gtest.h>
#include <vlq/vlq.h>
#include <vector>

#define DEFINE_TEST_LVLQ(NAME, SIZE, VALUE, ...) \
TEST(VLQ, NAME) \
{ \
    uint ## SIZE ## _t expected_value = VALUE; \
    std::vector<uint8_t> expected_encoded = __VA_ARGS__; \
    int expected_byte_count = expected_encoded.size(); \
 \
    int size = lvlq_encoded_size_ ## SIZE(VALUE); \
    EXPECT_EQ(expected_byte_count, size); \
 \
    std::vector<uint8_t> actual_encoded(20); \
    int bytes_written = lvlq_encode_ ## SIZE(expected_value, actual_encoded.data(), actual_encoded.size()); \
    if(bytes_written >= 0) \
    { \
        actual_encoded.resize(bytes_written); \
    } \
    EXPECT_EQ(expected_byte_count, bytes_written); \
    EXPECT_EQ(expected_encoded, actual_encoded); \
    uint ## SIZE ## _t actual_value = 0; \
    int bytes_read = lvlq_decode_ ## SIZE(&actual_value, expected_encoded.data(), expected_encoded.size()); \
    EXPECT_EQ(bytes_written, bytes_read); \
    EXPECT_EQ(expected_value, actual_value); \
}

#define DEFINE_TEST_LVLQ_DECODE_INITIAL(NAME, SIZE, INITIAL_VALUE, EXPECTED_VALUE, ...) \
TEST(VLQ, NAME) \
{ \
    uint ## SIZE ## _t expected_value = EXPECTED_VALUE; \
    std::vector<uint8_t> encoded = __VA_ARGS__; \
    uint ## SIZE ## _t actual_value = INITIAL_VALUE; \
    lvlq_decode_ ## SIZE(&actual_value, encoded.data(), encoded.size()); \
    EXPECT_EQ(expected_value, actual_value); \
}

#define DEFINE_TEST_RVLQ(NAME, SIZE, VALUE, ...) \
TEST(VLQ, NAME) \
{ \
    uint ## SIZE ## _t expected_value = VALUE; \
    std::vector<uint8_t> expected_encoded = __VA_ARGS__; \
    int expected_byte_count = expected_encoded.size(); \
 \
    int size = rvlq_encoded_size_ ## SIZE(VALUE); \
    EXPECT_EQ(expected_byte_count, size); \
 \
    std::vector<uint8_t> actual_encoded(20); \
    int bytes_written = rvlq_encode_ ## SIZE(expected_value, actual_encoded.data(), actual_encoded.size()); \
    if(bytes_written >= 0) \
    { \
        actual_encoded.resize(bytes_written); \
    } \
    EXPECT_EQ(expected_byte_count, bytes_written); \
    EXPECT_EQ(expected_encoded, actual_encoded); \
    uint ## SIZE ## _t actual_value = 0; \
    int bytes_read = rvlq_decode_ ## SIZE(&actual_value, expected_encoded.data(), expected_encoded.size()); \
    EXPECT_EQ(bytes_written, bytes_read); \
    EXPECT_EQ(expected_value, actual_value); \
}

#define DEFINE_TEST_RVLQ_DECODE_INITIAL(NAME, SIZE, INITIAL_VALUE, EXPECTED_VALUE, ...) \
TEST(VLQ, NAME) \
{ \
    uint ## SIZE ## _t expected_value = EXPECTED_VALUE; \
    std::vector<uint8_t> encoded = __VA_ARGS__; \
    uint ## SIZE ## _t actual_value = INITIAL_VALUE; \
    rvlq_decode_ ## SIZE(&actual_value, encoded.data(), encoded.size()); \
    EXPECT_EQ(expected_value, actual_value); \
}


DEFINE_TEST_LVLQ(left_64_0, 64, 0, {0x00})
DEFINE_TEST_LVLQ(left_64_1, 64, 1, {0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00})
DEFINE_TEST_LVLQ(left_64_2, 64, 2, {0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00})
DEFINE_TEST_LVLQ(left_64_8000000000000000, 64, 0x8000000000000000ULL, {0x40})
DEFINE_TEST_LVLQ(left_64_4000000000000000, 64, 0x4000000000000000ULL, {0x20})
DEFINE_TEST_LVLQ(left_64_2000000000000000, 64, 0x2000000000000000ULL, {0x10})
DEFINE_TEST_LVLQ(left_64_1000000000000000, 64, 0x1000000000000000ULL, {0x08})
DEFINE_TEST_LVLQ(left_64_1234567812345678, 64, 0x1234567812345678ULL, {0xbc, 0xd6, 0xe8, 0xc8, 0xc0, 0xe7, 0x8a, 0x8d, 0x09})
DEFINE_TEST_LVLQ_DECODE_INITIAL(left_continuation_32, 32, 0x80, 0x02000001, {0x01})
DEFINE_TEST_LVLQ_DECODE_INITIAL(left_continuation_64, 64, 0x80, 0x0200000000000001ULL, {0x01})

DEFINE_TEST_RVLQ(right_64_0, 64, 0, {0x00})
DEFINE_TEST_RVLQ(right_64_1, 64, 1, {0x01})
DEFINE_TEST_RVLQ(right_64_2, 64, 2, {0x02})
DEFINE_TEST_RVLQ(right_64_8000000000000000, 64, 0x8000000000000000ULL, {0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00})
DEFINE_TEST_RVLQ(right_64_4000000000000000, 64, 0x4000000000000000ULL, {0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00})
DEFINE_TEST_RVLQ(right_64_2000000000000000, 64, 0x2000000000000000ULL, {0xa0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00})
DEFINE_TEST_RVLQ(right_64_1000000000000000, 64, 0x1000000000000000ULL, {0x90, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00})
DEFINE_TEST_RVLQ(right_64_1234567812345678, 64, 0x1234567812345678ULL, {0x92, 0x9a, 0x95, 0xcf, 0x81, 0x91, 0xd1, 0xac, 0x78})
DEFINE_TEST_RVLQ_DECODE_INITIAL(right_continuation_32, 32, 0x01, 0x81, {0x01})
DEFINE_TEST_RVLQ_DECODE_INITIAL(right_continuation_64, 64, 0x01, 0x81, {0x01})

DEFINE_TEST_RVLQ(example_rvlq_2000000, 32, 2000000, {0xfa, 0x89, 0x00})
DEFINE_TEST_LVLQ(example_lvlq_19400000, 32, 0x19400000, {0xd0, 0x0c})
DEFINE_TEST_RVLQ(example_rvlq_05, 32, 0x05, {0x05})
DEFINE_TEST_RVLQ(example_rvlq_0d295a, 32, 0x0d295a, {0xb4, 0xd2, 0x5a})
DEFINE_TEST_LVLQ(example_lvlq_b549a000, 32, 0xb549a000, {0xb4, 0xd2, 0x5a})
