Variable Length Quantity Encoding
=================================

Variable Length Quantity (VLQ) encoding is an unsigned integer compression scheme designed for the MIDI file format.


### Basic Operation

An unsigned integer is encoded into groups of 7 bits, then encoded into bytes containing the 7 bits of data + a high "continuation" bit. A continuation bit of `1` indicates that the encoding continues on to the next byte. Any high groups containing all zero bits are not encoded since they're not needed in order to reconstruct the original value.

Value (32-bit, as binary): `0000 0000 0000 0xxx xxyy yyyy yzzz zzzz`

| High Bit | Low Bits  | Notes               |
| -------- | --------- | ------------------- |
|      `1` | `00xxxxx` | High 7 bits of data |
|      `1` | `yyyyyyy` | Next 7 bits of data |
|      `0` | `zzzzzzz` | Low 7 bits of data  |

The encoded bytes are stored in big endian order.


### Examples

#### Example: Encoding the value 2000000 to a VLQ

Get the binary representation:

    2000000 = 0x1e8480 = 0001 1110 1000 0100 1000 0000

Arrange into groups of 7 bits:

    000 1111010 0001001 0000000

Drop the highest group if it's all zero bits:

    1111010 0001001 0000000

Add a continuation bit to all but the lowest group:

    (1) 1111010  (1) 0001001  (0) 0000000
       11111010     10001001     00000000
           0xfa         0x89         0x00

Store in big endian order:

    [fa 89 00]

#### Example: Decoding a VLQ from the sequence `[05 0f 4a e4 aa]`

Byte 0 (0x05) has the high bit cleared, so we are done. The value is 5, and the bytes following it (`[0f 4a e4 aa]`) are not part of the VLQ.

#### Example: Decoding a VLQ from the sequence `[b4 d2 5a 91 ff]`

##### Separate the continuation bits from the data bits:

| Byte 0        | Byte 1        | Byte 2        |
| ------------- | ------------- | ------------- |
|        `0xb4` |        `0xd2` |        `0x5a` |
|    `10110100` |    `11010010` |    `01011010` |
| `1` `0110100` | `1` `1010010` | `0` `1011010` |

Byte 2 (0x5a) has its high bit cleared, so the bytes following it (`[91 ff]`) are not part of the VLQ.

##### Concatenate the 7-bit groups in big endian order:

    Byte 0     Byte 1     Byte 2
    0110100 ++ 1010010 ++ 1011010
    = 011010010100101011010
    = 01101 00101001 01011010
    =  0x0d     0x29     0x5a
    = 0x0d295a



Reverse VLQ
-----------

Reverse VLQ operates similarly to normal VLQ encoding, except that the encoded bytes are written in reversed order, starting at the **end** of the document:

Normal VLQ:

    [fa 89 00 xx xx xx xx xx] = 2000000

Reverse VLQ:

    [xx xx xx xx xx 00 89 fa] = 2000000

When decoding a reverse VLQ value, we start at the last byte of the document and read backwards until we encounter a byte with the high "continuation" bit cleared.

Reverse VLQ is useful for storing a VLQ value at the end of a document when it would be inconvenient or impossible to reach the value from the beginning of the document.



Why not Varint?
---------------

The most popular variable length unsigned integer encoding today is the VLQ variant in [Google's Protocol Buffers](https://developers.google.com/protocol-buffers/docs/encoding), which is identical except that it stores the encoded bytes in little endian order rather than big endian. However, big endian ordering allows for progressive decoding of values. For example:

Received buffers: `[b4 d2]`, `[5a 91 ff]`

Since the encoded integer spans two received buffers, you cannot decode it in one shot. When the bytes are encoded in big endian order, you can simply continue the decoding algorithm from one buffer to the next:

    accumulator = (accumulator << 7) | (next_byte&0x7f)

In this case:
* After buffer 0, byte 0 (0xb4): accumulator = 0x34
* After buffer 0, byte 1 (0xd2): accumulator = 0x1a52
* After buffer 1, byte 0 (0x5a): accumulator = 0xd295a

If the bytes were in little endian order, you couldn't do this, and would instead have to store all of the encoded bytes in a temporary buffer first.



License
-------

Copyright (c) 2019 Karl Stenerud. All rights reserved.

Distributed under the Creative Commons Attribution License: https://creativecommons.org/licenses/by/4.0/legalcode
License deed: https://creativecommons.org/licenses/by/4.0/
