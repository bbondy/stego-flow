#include "api.h"
#include "bmp.h"
#include "gif.h"
#include "jpg.h"
#include "png.h"

#include "steganography.h"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool cond, const std::string& msg) {
    if (!cond) {
        throw std::runtime_error(msg);
    }
}

void testCapacity() {
    BMPImage tiny(1, 1, Color(0, 0, 0));
    BMPImage small(2, 2, Color(0, 0, 0));
    require(stego::Steganography::capacityBytes(tiny) == 0, "1x1 should have zero payload capacity");
    require(stego::Steganography::capacityBytes(small) == 0, "2x2 should have zero payload capacity at 1 bit/channel");
    require(stego::Steganography::capacityBytes(small, 8) == 8, "2x2 with 8 bits/channel should have 8 bytes capacity");
}

void testRoundtripPerFormatObject(std::uint8_t bitsPerChannel) {
    const std::string msg = "Hello world";

    BMPImage bmp = api::createSmiley256BMP();
    PNGImage png = api::createSmiley256PNG();
    JPGImage jpg = api::createSmiley256JPG();
    GIFImage gif = api::createSmiley256GIF();

    {
        stego::Steganography s(bmp, bitsPerChannel);
        require(s.encodeMessage(msg), "BMP encode failed");
        require(s.decodeMessage() == msg, "BMP decode failed");
    }
    {
        stego::Steganography s(png, bitsPerChannel);
        require(s.encodeMessage(msg), "PNG encode failed");
        require(s.decodeMessage() == msg, "PNG decode failed");
    }
    {
        stego::Steganography s(jpg, bitsPerChannel);
        require(s.encodeMessage(msg), "JPG object encode failed");
        require(s.decodeMessage() == msg, "JPG object decode failed");
    }
    {
        stego::Steganography s(gif, bitsPerChannel);
        require(s.encodeMessage(msg), "GIF encode failed");
        require(s.decodeMessage() == msg, "GIF decode failed");
    }
}

void testPersistedLosslessRoundtrip() {
    const std::string msg = "Hello world";
    const std::uint8_t bitsPerChannel = 2;
    const std::string outDir = "build/output/test-images";
    std::filesystem::create_directories(outDir);

    PNGImage img = api::createSmiley256PNG();
    stego::Steganography writer(img, bitsPerChannel);
    require(writer.encodeMessage(msg), "Persisted PNG encode failed");
    require(img.save(outDir + "/stego_test.png"), "Failed to save stego_test.png");

    PNGImage loaded = PNGImage::load(outDir + "/stego_test.png");
    stego::Steganography reader(loaded, bitsPerChannel);
    require(reader.decodeMessage() == msg, "Persisted PNG decode failed");
}

void testInvalidBitDepth() {
    PNGImage img = api::createSmiley256PNG();

    bool threwLow = false;
    try {
        stego::Steganography invalid(img, 0);
    } catch (const std::invalid_argument&) {
        threwLow = true;
    }
    require(threwLow, "bitsPerChannel=0 should throw");

    bool threwHigh = false;
    try {
        stego::Steganography invalid(img, 9);
    } catch (const std::invalid_argument&) {
        threwHigh = true;
    }
    require(threwHigh, "bitsPerChannel=9 should throw");
}
} // namespace

int main() {
    try {
        testCapacity();
        testRoundtripPerFormatObject(1);
        testRoundtripPerFormatObject(2);
        testRoundtripPerFormatObject(4);
        testPersistedLosslessRoundtrip();
        testInvalidBitDepth();
        std::cout << "stego-flow tests passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "stego-flow test failure: " << ex.what() << "\n";
        return 1;
    }
}
