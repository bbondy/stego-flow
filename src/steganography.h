#ifndef STEGO_FLOW_STEGANOGRAPHY_H
#define STEGO_FLOW_STEGANOGRAPHY_H

#include "image.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace stego {
class Steganography {
public:
    explicit Steganography(RasterImage& image, std::uint8_t bitsPerChannel = 1);

    static std::size_t capacityBytes(const RasterImage& image, std::uint8_t bitsPerChannel = 1);
    bool encodeMessage(const std::string& message);
    std::string decodeMessage() const;

private:
    RasterImage& m_image;
    std::uint8_t m_bitsPerChannel;
};
} // namespace stego

#endif
