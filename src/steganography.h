#ifndef STEGO_FLOW_STEGANOGRAPHY_H
#define STEGO_FLOW_STEGANOGRAPHY_H

#include "image.h"

#include <cstddef>
#include <string>

namespace stego {
class Steganography {
public:
    explicit Steganography(RasterImage& image);

    static std::size_t capacityBytes(const RasterImage& image);
    bool encodeMessage(const std::string& message);
    std::string decodeMessage() const;

private:
    RasterImage& m_image;
};
} // namespace stego

#endif
