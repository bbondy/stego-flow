#include "api.h"
#include "bmp.h"
#include "gif.h"
#include "jpg.h"
#include "png.h"

#include "steganography.h"

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
    require(stego::Steganography::capacityBytes(tiny) == 0, "1x1 should have zero payload capacity");
}

void testRoundtripPerFormatObject() {
    const std::string msg = "Hello world";

    BMPImage bmp = api::createSmiley256BMP();
    PNGImage png = api::createSmiley256PNG();
    JPGImage jpg = api::createSmiley256JPG();
    GIFImage gif = api::createSmiley256GIF();

    {
        stego::Steganography s(bmp);
        require(s.encodeMessage(msg), "BMP encode failed");
        require(s.decodeMessage() == msg, "BMP decode failed");
    }
    {
        stego::Steganography s(png);
        require(s.encodeMessage(msg), "PNG encode failed");
        require(s.decodeMessage() == msg, "PNG decode failed");
    }
    {
        stego::Steganography s(jpg);
        require(s.encodeMessage(msg), "JPG object encode failed");
        require(s.decodeMessage() == msg, "JPG object decode failed");
    }
    {
        stego::Steganography s(gif);
        require(s.encodeMessage(msg), "GIF encode failed");
        require(s.decodeMessage() == msg, "GIF decode failed");
    }
}

void testPersistedLosslessRoundtrip() {
    const std::string msg = "Hello world";
    const std::string outDir = "build/output/test-images";
    std::filesystem::create_directories(outDir);

    PNGImage img = api::createSmiley256PNG();
    stego::Steganography writer(img);
    require(writer.encodeMessage(msg), "Persisted PNG encode failed");
    require(img.save(outDir + "/stego_test.png"), "Failed to save stego_test.png");

    PNGImage loaded = PNGImage::load(outDir + "/stego_test.png");
    stego::Steganography reader(loaded);
    require(reader.decodeMessage() == msg, "Persisted PNG decode failed");
}
} // namespace

int main() {
    try {
        testCapacity();
        testRoundtripPerFormatObject();
        testPersistedLosslessRoundtrip();
        std::cout << "stego-flow tests passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "stego-flow test failure: " << ex.what() << "\n";
        return 1;
    }
}
