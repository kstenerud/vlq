#include <gtest/gtest.h>
#include <vlq/vlq.h>
#include <stdio.h>

static void print_buffer(uint8_t* buffer, int length)
{
    for(int i = 0; i < length; i++)
    {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

static void demonstrate_rvlq_write()
{
    uint8_t buffer[10];
    uint64_t value = 0x4aa61f24f4ul;
    int encode_count = rvlq_encode_64(value, buffer, sizeof(buffer));
    if(encode_count == 0)
    {
        // There wasn't enough room in buffer
    }
    else
    {
        print_buffer(buffer, encode_count); // Prints 89 aa b0 fc c9 74
    }
}

static void demonstrate_rvlq_read()
{
    uint8_t buffer[] = {0x86, 0xd2, 0x17};
    uint64_t value = 0;
    int decode_count = rvlq_decode_64(&value, buffer, sizeof(buffer));
    if(decode_count == 0)
    {
        // VLQ wasn't terminated.
    }
    else
    {
        printf("%016lx\n", value); // Prints 000000000001a917
    }
}

TEST(Readme, example)
{
	demonstrate_rvlq_write();
    demonstrate_rvlq_read();
}
