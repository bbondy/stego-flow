# stego-flow

Steganography-focused companion project for `image-flow`.

This project keeps LSB message embedding/extraction logic isolated while
using `image-flow` as a git dependency for image types/codecs.

`Steganography` now supports configurable LSB depth per RGB color component
(`bitsPerChannel`, range `1..8`, default `1`).

## CLI

Build:

```sh
make
```

Commands:

```sh
# Demo roundtrip with generated image
./stego_demo demo [bitsPerChannel]

# Embed arbitrary binary payload file into an image from disk
./stego_demo embed-file <input-image> <payload-file> <output-image> [bitsPerChannel]

# Extract payload bytes from an image to a file on disk
./stego_demo extract-file <input-image> <output-file> [bitsPerChannel]
```

Supported image extensions are `.png`, `.bmp`, `.jpg`/`.jpeg`, and `.gif`.
