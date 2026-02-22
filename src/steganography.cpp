#include "steganography.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
constexpr std::uint8_t kMinBitsPerChannel = 1;
constexpr std::uint8_t kMaxBitsPerChannel = 8;

void validateBitsPerChannel(std::uint8_t bitsPerChannel) {
    if (bitsPerChannel < kMinBitsPerChannel || bitsPerChannel > kMaxBitsPerChannel) {
        throw std::invalid_argument("bitsPerChannel must be in the range [1, 8]");
    }
}

std::size_t capacityBits(const RasterImage& image, std::uint8_t bitsPerChannel) {
    return static_cast<std::size_t>(image.width()) * static_cast<std::size_t>(image.height()) * 3U * bitsPerChannel;
}

void pushBits(std::vector<std::uint8_t>& bits, std::uint32_t value, int count) {
    for (int i = count - 1; i >= 0; --i) {
        bits.push_back(static_cast<std::uint8_t>((value >> i) & 1U));
    }
}

std::uint32_t consumeBits(const std::vector<std::uint8_t>& bits, std::size_t& bitIndex, std::uint8_t count) {
    std::uint32_t value = 0;
    std::uint8_t consumed = 0;
    for (; consumed < count && bitIndex < bits.size(); ++consumed) {
        value = static_cast<std::uint32_t>((value << 1U) | bits[bitIndex++]);
    }
    if (consumed < count) {
        value <<= (count - consumed);
    }
    return value;
}

void setChannelLSBs(std::uint8_t& channel, std::uint32_t value, std::uint8_t count) {
    const std::uint16_t mask = static_cast<std::uint16_t>((1U << count) - 1U);
    channel = static_cast<std::uint8_t>((channel & static_cast<std::uint8_t>(~mask)) | (value & mask));
}

std::uint32_t getChannelLSBs(std::uint8_t channel, std::uint8_t count) {
    const std::uint32_t mask = (1U << count) - 1U;
    return static_cast<std::uint32_t>(channel) & mask;
}
} // namespace

namespace stego {
Steganography::Steganography(RasterImage& image, std::uint8_t bitsPerChannel)
    : m_image(image), m_bitsPerChannel(bitsPerChannel) {
    validateBitsPerChannel(bitsPerChannel);
}

std::size_t Steganography::capacityBytes(const RasterImage& image, std::uint8_t bitsPerChannel) {
    validateBitsPerChannel(bitsPerChannel);
    const std::size_t bits = capacityBits(image, bitsPerChannel);
    if (bits <= 32) {
        return 0;
    }
    return (bits - 32) / 8;
}

bool Steganography::encodeMessage(const std::string& message) {
    const std::size_t cap = capacityBytes(m_image, m_bitsPerChannel);
    if (message.size() > cap) {
        return false;
    }
    if (message.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        return false;
    }

    std::vector<std::uint8_t> bits;
    bits.reserve(32 + message.size() * 8);

    pushBits(bits, static_cast<std::uint32_t>(message.size()), 32);
    for (unsigned char c : message) {
        pushBits(bits, static_cast<std::uint32_t>(c), 8);
    }

    std::size_t bitIndex = 0;
    for (int y = 0; y < m_image.height() && bitIndex < bits.size(); ++y) {
        for (int x = 0; x < m_image.width() && bitIndex < bits.size(); ++x) {
            Color px = m_image.getPixel(x, y);

            if (bitIndex < bits.size()) {
                setChannelLSBs(px.r, consumeBits(bits, bitIndex, m_bitsPerChannel), m_bitsPerChannel);
            }
            if (bitIndex < bits.size()) {
                setChannelLSBs(px.g, consumeBits(bits, bitIndex, m_bitsPerChannel), m_bitsPerChannel);
            }
            if (bitIndex < bits.size()) {
                setChannelLSBs(px.b, consumeBits(bits, bitIndex, m_bitsPerChannel), m_bitsPerChannel);
            }

            m_image.setPixel(x, y, px);
        }
    }

    return true;
}

std::string Steganography::decodeMessage() const {
    const std::size_t totalBits = capacityBits(m_image, m_bitsPerChannel);
    if (totalBits < 32) {
        throw std::runtime_error("Image too small for steganography header");
    }

    std::vector<std::uint8_t> bits;
    bits.reserve(totalBits);

    for (int y = 0; y < m_image.height(); ++y) {
        for (int x = 0; x < m_image.width(); ++x) {
            const Color& px = m_image.getPixel(x, y);
            pushBits(bits, getChannelLSBs(px.r, m_bitsPerChannel), m_bitsPerChannel);
            pushBits(bits, getChannelLSBs(px.g, m_bitsPerChannel), m_bitsPerChannel);
            pushBits(bits, getChannelLSBs(px.b, m_bitsPerChannel), m_bitsPerChannel);
        }
    }

    std::uint32_t messageLen = 0;
    for (int i = 0; i < 32; ++i) {
        messageLen = static_cast<std::uint32_t>((messageLen << 1) | bits[static_cast<std::size_t>(i)]);
    }

    const std::size_t neededBits = 32 + static_cast<std::size_t>(messageLen) * 8;
    if (neededBits > bits.size()) {
        throw std::runtime_error("Invalid steganography payload length");
    }

    std::string message;
    message.resize(messageLen);

    std::size_t bitPos = 32;
    for (std::uint32_t i = 0; i < messageLen; ++i) {
        std::uint8_t value = 0;
        for (int b = 0; b < 8; ++b) {
            value = static_cast<std::uint8_t>((value << 1) | bits[bitPos++]);
        }
        message[static_cast<std::size_t>(i)] = static_cast<char>(value);
    }

    return message;
}
} // namespace stego
