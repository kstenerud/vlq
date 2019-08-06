Variable Length Quantity Encoding
=================================

Variable Length Quantity (VLQ) encoding is an unsigned integer compression scheme originally designed for the MIDI file format. This specification expands upon it slightly by allowing encoding from the "left" or "right" side of the quantity (the original MIDI version is "right" oriented).



Encoding
--------

A value is encoded into groups of 7 bits, then encoded into bytes containing the 7 bits of data + a high "continuation" bit. A continuation bit of `1` indicates that the encoding continues on to the next byte. Any trailing groups containing all zero bits are not encoded since they're not needed to reconstruct the original value.


### Orientation

Grouping can be done either from the "left" side (**LVLQ**) or from the "right" side (**RVLQ**), depending on which side of the values in your data the bits tend to cluster at. The endianness is determined by the encoding direction, chosen to allow decoding of the value without requiring any additional state, even when the encoded value is split across multiple buffer reads.


### Illustration: Right-heavy 32-bit value encoded as RVLQ

    00000000 00000abc defghijk lmnopqrs

Break into 7 bit groups, starting from the right:

    0000 0000000 00abcde fghijkl mnopqrs

Write in big endian order, omitting empty groups on the left:

| Continuation Bit | Data Bits  | Notes               |
| ---------------- | ---------- | ------------------- |
|              `1` | `00abcde`  | High 5 bits of data |
|              `1` | `fghijkl`  | Next 7 bits of data |
|              `0` | `mnopqrs`  | Low 7 bits of data  |

    = 100abcde 1fghijkl 0mnopqrs

To decode:

    loop:
        get next byte
        accumulator = (accumulator << 7) | (next_byte & 0x7f)
        if (next_byte & 0x80) == 0:
            break


### Illustration: Left-heavy 32-bit value encoded as LVLQ

    abcdefgh ijklmnop qrs00000 00000000

Break into 7 bit groups, starting from the left:

    abcdefg hijklmn opqrs00 0000000 0000

Write in little endian order, omitting empty groups on the right:

| Continuation Bit | Data Bits  | Notes               |
| ---------------- | ---------- | ------------------- |
|              `1` | `opqrs00`  | Low 5 bits of data  |
|              `1` | `hijklmn`  | Next 7 bits of data |
|              `0` | `abcdefg`  | High 7 bits of data |

    = 1opqrs00 1hijklmn 0abcdefg

To decode:

    loop:
        get next byte
        accumulator = (accumulator >> 7) | (next_byte << (sizeof(accumulator) - 7))
        if (next_byte & 0x80) == 0:
            break



Examples
--------

### Example: Encoding the 32-bit value 2000000 to a RVLQ

Get the binary representation:

    2000000 = 0x1e8480 = 0001 1110 1000 0100 1000 0000

Arrange into groups of 7 bits from the right:

    000 1111010 0001001 0000000

Drop any high groups that are all zero bits:

    1111010 0001001 0000000

Add a continuation bit to all but the lowest group:

    (1) 1111010  (1) 0001001  (0) 0000000
       11111010     10001001     00000000
           0xfa         0x89         0x00

Store in big endian order:

    [fa 89 00]


### Example: Encoding the 32-bit value 0x19400000 to a LVLQ

Get the binary representation:

    0x19400000 = 0001 1001 0100 0000 0000 0000 0000 0000

Arrange into groups of 7 bits from the left:

    0001100 1010000 0000000 0000000 0000

Drop any low groups that are all zero bits:

    0001100 1010000

Add a continuation bit to all but the highest group:

    (0) 0001100 (1) 1010000
       00001100    11010000
           0x0c        0xd0

Store in little endian order:

    [d0 0c]


### Example: Decoding a RVLQ from the sequence `[05 0f 4a e4 aa]`

Byte 0 (0x05) has the high bit cleared, so we are done. The value is 5, and the bytes following it (`[0f 4a e4 aa]`) are not part of the VLQ.


### Example: Decoding a RVLQ from the sequence `[b4 d2 5a 91 ff]`

#### Separate the continuation bits from the data bits:

| Byte 0        | Byte 1        | Byte 2        |
| ------------- | ------------- | ------------- |
|        `0xb4` |        `0xd2` |        `0x5a` |
|    `10110100` |    `11010010` |    `01011010` |
| `1` `0110100` | `1` `1010010` | `0` `1011010` |

Byte 2 (0x5a) has its high bit cleared, so the bytes following it (`[91 ff]`) are not part of the VLQ.

#### Concatenate the 7-bit groups in big endian order:

    Byte 0     Byte 1     Byte 2
    0110100 ++ 1010010 ++ 1011010
    = 011010010100101011010
    = 01101 00101001 01011010
    =  0x0d     0x29     0x5a
    = 0x0d295a


### Example: Decoding a LVLQ from the sequence `[b4 d2 5a 91 ff]`

#### Separate the continuation bits from the data bits:

| Byte 0        | Byte 1        | Byte 2        |
| ------------- | ------------- | ------------- |
|        `0xb4` |        `0xd2` |        `0x5a` |
|    `10110100` |    `11010010` |    `01011010` |
| `1` `0110100` | `1` `1010010` | `0` `1011010` |

Byte 2 (0x5a) has its high bit cleared, so the bytes following it (`[91 ff]`) are not part of the VLQ.

#### Concatenate the 7-bit groups in little endian order:

    Byte 2     Byte 1     Byte 0
    1011010 ++ 1010010 ++ 0110100
    = 101101010100100110100
    = 10110101 01001001 10100 (000)
    =     0xb5     0x49      0xa0
    = 0xb549a000 (as a 32-bit value)



Why not Varint?
---------------

The most popular variable length unsigned integer encoding today is the VLQ variant `varint` in [Google's Protocol Buffers](https://developers.google.com/protocol-buffers/docs/encoding). Varint stores the groups right-oriented and in little endian order, which increases the codec complexity unnecessarily.

Example:

Received over 2 buffer reads: `[84 d2]`, `[ff 91 51 ...]`

Since the encoded integer spans two received buffers, you cannot decode it in one shot. If the groups are written in big endian order, you can continue the simple algorithm of shifting and adding over to the next buffer:

    accumulator = (accumulator << 7) | (next_byte&0x7f)

Operations:
* Read buffer 0, byte 0 (0x84, data = 0x04): `accumulator` = 0x00000004
* Read buffer 0, byte 1 (0xd2, data = 0x52): `accumulator` = 0x00000252
* End of buffer 0. Return `accumulator` and end status (not complete).
* Read buffer 1, byte 0 (0xff, data = 0x7f): `accumulator` = 0x0001297f
* Read buffer 1, byte 1 (0x91, data = 0x11): `accumulator` = 0x0094bf91
* Read buffer 1, byte 2 (0x51, data = 0x51): `accumulator` = 0x4a5fc8d1
* End of VLQ. Return `accumulator` and end status (complete).

With varint, the groups are written in little endian order, so you would have to keep track of extra state information across calls to make sure subsequent groups are added to the proper bit position. Also, `next_byte` now has to be extended to the width of the accumulator, even if the CPU has smaller/faster opcodes for mixed sized operand combinations:

Received over 2 buffer reads: `[d1 91]`, `[ff d2 04 ...]`

    accumulator = accumulator | ((uintXXX_t)(next_byte&0x7f) << shift_amount)
    shift_amount += 7

Operations
* Read buffer 0, byte 0 (0xd1, data = 0x51): `accumulator` = 0x00000051, `shift_amount` = 7
* Read buffer 0, byte 1 (0x91, data = 0x11): `accumulator` = 0x000008d1, `shift_amount` = 14
* End of buffer 0. Return `accumulator`, `shift_amount`, and end status (not complete).
* Read buffer 1, byte 0 (0xff, data = 0x7f): `accumulator` = 0x001fc8d1, `shift_amount` = 21
* Read buffer 1, byte 1 (0xd2, data = 0x52): `accumulator` = 0x0a5fc8d1, `shift_amount` = 28
* Read buffer 1, byte 1 (0x04, data = 0x04): `accumulator` = 0x4a5fc8d1, `shift_amount` = 35
* End of VLQ. Return `accumulator`, `shift_amount`, and end status (complete).

There are also potential security implications: With big endian ordered groups, if the decoder encountered an encoded value that was bigger than its accumulator, the excess bits would simply be shifted off the end. If you didn't guard gainst this, you'd have an incorrect value, but otherwise no harm done. With little endian ordered groups, you're incrementing `shift_amount`, which would eventually trigger undefined behavior in languages such as C and C++ once it exceeds the bit width of the accumulator.



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
