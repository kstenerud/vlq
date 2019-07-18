VLQ Reference Implementation
============================

Codec for variable length quantity.

Implemented as a header only ([`vlq.h`](https://github.com/kstenerud/vlq/blob/master/reference-implementation/include/vlq/vlq.h)).



Header Dependencies
-------------------

 * stdint.h: For standard integer types


Usage
-----

```c
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
```



Requirements
------------

  * Meson 0.49 or newer
  * Ninja 1.8.2 or newer
  * A C compiler
  * A C++ compiler (for the tests)



Building
--------

    meson build
    ninja -C build



Running Tests
-------------

    ninja -C build test

For the full report:

    ./build/run_tests



Installing
----------

    ninja -C build install



License
-------

Copyright 2019 Karl Stenerud

Released under MIT License https://opensource.org/licenses/MIT
