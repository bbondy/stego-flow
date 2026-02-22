# stego-flow

Steganography-focused companion project for `image-flow`.

This project keeps LSB message embedding/extraction logic isolated while
using `image-flow` as a git dependency for image types/codecs.

`Steganography` now supports configurable LSB depth per RGB color component
(`bitsPerChannel`, range `1..8`, default `1`).
