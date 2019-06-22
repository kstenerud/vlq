Variable Length Quantity
========================

Variable Length Quantity (VLQ) is an encoding scheme to compress unsigned integers. It was originally designed for the MIDI file format, but has found uses in many areas since.

VLQ encodes smaller integer values into less bytes than larger values. Most real-world applications tend to store more small values than large, so the space savings can be substantial.



Specification
-------------

 * [Variable Length Quantity](vlq-specification.md)



Implementations
---------------

* [Go implementation](https://github.com/kstenerud/go-vlq)



License
-------

Copyright 2019 Karl Stenerud

Specification released under Creative Commons Attribution 4.0 International Public License.
