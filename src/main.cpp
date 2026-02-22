#include "api.h"
#include "png.h"

#include "steganography.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    try {
        const std::string hidden = "Hello world";

        PNGImage image = api::createSmiley256PNG();
        stego::Steganography encoder(image);
        if (!encoder.encodeMessage(hidden)) {
            std::cerr << "Failed to encode message\n";
            return 1;
        }
        if (!image.save("stego_message.png")) {
            std::cerr << "Failed to save stego_message.png\n";
            return 1;
        }

        PNGImage loaded = PNGImage::load("stego_message.png");
        stego::Steganography decoder(loaded);
        const std::string extracted = decoder.decodeMessage();

        std::cout << "Extracted: " << extracted << "\n";
        return extracted == hidden ? 0 : 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
