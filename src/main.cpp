#include "api.h"
#include "png.h"

#include "steganography.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    try {
        const std::string hidden = "Hello world";
        const std::string outDir = "build/output/images";
        std::uint8_t bitsPerChannel = 1;
        if (argc > 2) {
            std::cerr << "Usage: " << argv[0] << " [bitsPerChannel]\n";
            return 1;
        }
        if (argc == 2) {
            const int parsed = std::stoi(argv[1]);
            if (parsed < 1 || parsed > 8) {
                std::cerr << "bitsPerChannel must be in the range [1, 8]\n";
                return 1;
            }
            bitsPerChannel = static_cast<std::uint8_t>(parsed);
        }

        std::filesystem::create_directories(outDir);

        PNGImage image = api::createSmiley256PNG();
        stego::Steganography encoder(image, bitsPerChannel);
        if (!encoder.encodeMessage(hidden)) {
            std::cerr << "Failed to encode message\n";
            return 1;
        }
        if (!image.save(outDir + "/stego_message.png")) {
            std::cerr << "Failed to save stego_message.png\n";
            return 1;
        }

        PNGImage loaded = PNGImage::load(outDir + "/stego_message.png");
        stego::Steganography decoder(loaded, bitsPerChannel);
        const std::string extracted = decoder.decodeMessage();

        std::cout << "Extracted: " << extracted << " (bitsPerChannel=" << static_cast<int>(bitsPerChannel) << ")\n";
        return extracted == hidden ? 0 : 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
