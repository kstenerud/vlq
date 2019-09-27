Variable Length Quantity
========================

Variable Length Quantity (VLQ) encoding is an unsigned integer compression scheme originally designed for the MIDI file format. This specification expands upon it slightly by allowing encoding from the "left" (**LVLQ**) or "right" (**RVLQ**) side of the quantity (the original MIDI version is "right" oriented).

VLQ compresses leading (LVLQ) or trailing (RVLQ) zero bits, which can lead to substantial savings since the majority of integer values tend to have lots of leading/trailing zero bits, especially to the right side (because most values are small).



Specification
-------------

 * [Variable Length Quantity](vlq-specification.md)



Implementations
---------------

* [C implementation](https://github.com/kstenerud/c-vlq)
* [Go implementation](https://github.com/kstenerud/go-vlq)



License
-------

Copyright 2019 Karl Stenerud

Specification released under Creative Commons Attribution 4.0 International Public License.
