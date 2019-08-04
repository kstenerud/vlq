// Copyright 2018 Karl Stenerud
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef KS_VLQ_H
#define KS_VLQ_H

#include <stdint.h>
#include <stdbool.h>

//
// Code generator macros to create encode/decode functions which encode/decode
// values into/from VLQ groups, either from the "left" (LVLQ) or from the
// "right" (RVLQ).
//
// With "left" encoding, grouping starts from the msb side, zero groups are
// omitted from the lsb side, and groups are added in little endian order:
// [abcdefg] [abcdefg] [0000000] [0000000] [0000000] ...
//
// With "right" encoding, grouping starts from the lsb side, zero groups are
// omitted from the msb side, and groups are added in big endian order:
// ... [0000000] [0000000] [0000000] [abcdefg] [abcdefg]
//
// Encode returns the number of bytes encoded, or 0 if there wasn't enough room.
// Decode returns the number of bytes decoded, or 0 if the end was not found.
//
// Note: The decode value field is IN/OUT, meaning that it will "continue" decoding
// based on what's currently in value (shifting and adding bits to it).
//
#define DEFINE_VLQ_ENCODE_DECODE_FUNCTIONS(SIZE, TYPE) \
static int lvlq_encoded_size_ ## SIZE(TYPE value) \
{ \
    if(value == 0) \
    { \
        return 1; \
    } \
    int size = 0; \
    while(value > 0) \
    { \
        value <<= 7; \
        size++; \
    } \
    return size; \
} \
static int lvlq_encode_ ## SIZE(TYPE value, uint8_t* buffer, int buffer_length) \
{ \
    uint8_t const* end = buffer + buffer_length; \
    const int total_bits = sizeof(value) * 8; \
    const int group_count = total_bits / 7; \
    const int extra_bit_count = total_bits % 7; \
    const int extra_shift = (7 - extra_bit_count) % 7; \
    const unsigned extra_mask = (1 << extra_bit_count) - 1; \
    uint8_t* ptr = buffer; \
 \
    if(buffer_length == 0) \
    { \
        return 0; \
    } \
 \
    if(value == 0) \
    { \
        *ptr = 0; \
        return 1; \
    } \
 \
    int group_index = 0; \
 \
    uint8_t extra_bits = (uint8_t)((value & extra_mask) << extra_shift); \
    value >>= extra_bit_count; \
    if(extra_bits != 0) \
    { \
        *ptr++ = extra_bits | 0x80; \
    } \
    else \
    { \
        while((value & 0x7f) == 0) \
        { \
            group_index++; \
            value >>= 7; \
        } \
    } \
 \
    while(group_index < group_count) \
    { \
        if(ptr >= end) \
        { \
            return 0; \
        } \
        uint8_t next_byte = value & 0x7f; \
        if(group_index < group_count - 1) \
        { \
            next_byte |= 0x80; \
        } \
        *ptr++ = next_byte; \
        group_index++; \
        value >>= 7; \
    } \
 \
    return ptr - buffer; \
} \
 \
static int lvlq_decode_ ## SIZE(TYPE* value, const uint8_t* buffer, int buffer_length) \
{ \
    const int group_shift = sizeof(*value) * 8 - 7; \
    const uint8_t* end = buffer + buffer_length; \
    const uint8_t* ptr = buffer; \
    TYPE accumulator = *value; \
    bool terminated = false; \
 \
    while(ptr < end) \
    { \
        TYPE next_byte = *ptr++; \
        accumulator = (accumulator >> 7) | (next_byte << group_shift); \
        if((next_byte & 0x80) == 0) \
        { \
            terminated = true; \
            break; \
        } \
    } \
    if(!terminated) \
    { \
        return 0; \
    } \
    *value = accumulator; \
    return ptr - buffer; \
} \
static int rvlq_encoded_size_ ## SIZE(TYPE value) \
{ \
    if(value <= 0x7f) \
    { \
        return 1; \
    } \
    int size = 0; \
    while(value > 0) \
    { \
        value >>= 7; \
        size++; \
    } \
    return size; \
} \
static int rvlq_encode_ ## SIZE(TYPE value, uint8_t* buffer, int buffer_length) \
{ \
    uint8_t const* end = buffer + buffer_length; \
    const int total_bits = sizeof(value) * 8; \
    const int group_count = total_bits / 7; \
    const int extra_bit_count = total_bits % 7; \
    const int value_shift = total_bits - 7; \
    const int extra_shift = total_bits - extra_bit_count; \
    const TYPE group_mask = ((TYPE)0x7f) << value_shift; \
    uint8_t* ptr = buffer; \
 \
    if(buffer_length == 0) \
    { \
        return 0; \
    } \
 \
    if(value == 0) \
    { \
        *ptr = 0; \
        return 1; \
    } \
 \
    int group_index = 0; \
 \
    uint8_t extra_bits = (uint8_t)(value >> extra_shift); \
    value <<= extra_bit_count; \
    if(extra_bits != 0) \
    { \
        *ptr++ = extra_bits | 0x80; \
    } \
    else \
    { \
        while((value & group_mask) == 0) \
        { \
            group_index++; \
            value <<= 7; \
        } \
    } \
 \
    while(group_index < group_count) \
    { \
        if(ptr >= end) \
        { \
            return 0; \
        } \
        uint8_t next_byte = (value>>value_shift) & 0x7f; \
        if(group_index < group_count - 1) \
        { \
            next_byte |= 0x80; \
        } \
        *ptr++ = next_byte; \
        group_index++; \
        value <<= 7; \
    } \
 \
    return ptr - buffer; \
} \
 \
static int rvlq_decode_ ## SIZE(TYPE* value, const uint8_t* buffer, int buffer_length) \
{ \
    const uint8_t* end = buffer + buffer_length; \
    const uint8_t* ptr = buffer; \
    TYPE accumulator = *value; \
    bool terminated = false; \
 \
    while(ptr < end) \
    { \
        uint8_t next_byte = *ptr++; \
        accumulator = (accumulator << 7) | (next_byte&0x7f); \
        if((next_byte & 0x80) == 0) \
        { \
            terminated = true; \
            break; \
        } \
    } \
    if(!terminated) \
    { \
        return 0; \
    } \
    *value = accumulator; \
    return ptr - buffer; \
}


_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-function\"")

DEFINE_VLQ_ENCODE_DECODE_FUNCTIONS(32, uint32_t)
DEFINE_VLQ_ENCODE_DECODE_FUNCTIONS(64, uint64_t)

// Not ISO compliant. Enable by setting this #define
#ifdef VLQ_ENABLE_GCC_128_BIT
DEFINE_VLQ_ENCODE_DECODE_FUNCTIONS(128, unsigned __int128)
#endif

_Pragma("GCC diagnostic pop")

#endif // KS_VLQ_H
