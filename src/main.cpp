#include "example_api.h"
#include "bmp.h"
#include "gif.h"
#include "jpg.h"
#include "png.h"

#include "steganography.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::string toLower(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return s;
}

std::uint8_t parseBitsPerChannel(const std::string& value) {
    const int parsed = std::stoi(value);
    if (parsed < 1 || parsed > 8) {
        throw std::invalid_argument("bitsPerChannel must be in the range [1, 8]");
    }
    return static_cast<std::uint8_t>(parsed);
}

std::string fileExtensionLower(const std::string& path) {
    return toLower(std::filesystem::path(path).extension().string());
}

std::string readFileBytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open input file: " + path);
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void writeFileBytes(const std::string& path, const std::string& bytes) {
    const std::filesystem::path outPath(path);
    if (outPath.has_parent_path()) {
        std::filesystem::create_directories(outPath.parent_path());
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open output file: " + path);
    }
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        throw std::runtime_error("Failed to write output file: " + path);
    }
}

std::unique_ptr<RasterImage> loadImageByExtension(const std::string& path) {
    const std::string ext = fileExtensionLower(path);
    if (ext == ".png") {
        return std::make_unique<PNGImage>(PNGImage::load(path));
    }
    if (ext == ".bmp") {
        return std::make_unique<BMPImage>(BMPImage::load(path));
    }
    if (ext == ".jpg" || ext == ".jpeg") {
        return std::make_unique<JPGImage>(JPGImage::load(path));
    }
    if (ext == ".gif") {
        return std::make_unique<GIFImage>(GIFImage::load(path));
    }
    throw std::invalid_argument("Unsupported input image extension: " + ext);
}

bool saveImageByExtension(const RasterImage& image, const std::string& path) {
    const std::string ext = fileExtensionLower(path);

    if (ext == ".png") {
        const auto* typed = dynamic_cast<const PNGImage*>(&image);
        if (!typed) {
            throw std::runtime_error("Image type mismatch for .png output");
        }
        return typed->save(path);
    }
    if (ext == ".bmp") {
        const auto* typed = dynamic_cast<const BMPImage*>(&image);
        if (!typed) {
            throw std::runtime_error("Image type mismatch for .bmp output");
        }
        return typed->save(path);
    }
    if (ext == ".jpg" || ext == ".jpeg") {
        const auto* typed = dynamic_cast<const JPGImage*>(&image);
        if (!typed) {
            throw std::runtime_error("Image type mismatch for .jpg/.jpeg output");
        }
        return typed->save(path);
    }
    if (ext == ".gif") {
        const auto* typed = dynamic_cast<const GIFImage*>(&image);
        if (!typed) {
            throw std::runtime_error("Image type mismatch for .gif output");
        }
        return typed->save(path);
    }

    throw std::invalid_argument("Unsupported output image extension: " + ext);
}

int runDemo(std::uint8_t bitsPerChannel) {
    const std::string hidden = "Hello world";
    const std::string outDir = "build/output/images";
    std::filesystem::create_directories(outDir);

    PNGImage image = example_api::createSmiley256PNG();
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
}

int runEmbedFile(const std::string& inputImagePath, const std::string& payloadPath, const std::string& outputImagePath,
                 std::uint8_t bitsPerChannel) {
    auto image = loadImageByExtension(inputImagePath);
    const std::string payload = readFileBytes(payloadPath);

    stego::Steganography encoder(*image, bitsPerChannel);
    if (!encoder.encodeMessage(payload)) {
        std::cerr << "Payload too large for this image at bitsPerChannel=" << static_cast<int>(bitsPerChannel) << "\n";
        return 1;
    }

    if (!saveImageByExtension(*image, outputImagePath)) {
        std::cerr << "Failed to save output image: " << outputImagePath << "\n";
        return 1;
    }
    std::cout << "Embedded " << payload.size() << " bytes into " << outputImagePath << "\n";
    return 0;
}

int runExtractFile(const std::string& inputImagePath, const std::string& outputPath, std::uint8_t bitsPerChannel) {
    auto image = loadImageByExtension(inputImagePath);
    stego::Steganography decoder(*image, bitsPerChannel);
    const std::string payload = decoder.decodeMessage();
    writeFileBytes(outputPath, payload);
    std::cout << "Extracted " << payload.size() << " bytes to " << outputPath << "\n";
    return 0;
}

void printUsage(const char* argv0) {
    std::cerr << "Usage:\n"
              << "  " << argv0 << " demo [bitsPerChannel]\n"
              << "  " << argv0 << " embed-file <input-image> <payload-file> <output-image> [bitsPerChannel]\n"
              << "  " << argv0 << " extract-file <input-image> <output-file> [bitsPerChannel]\n";
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc <= 1) {
            return runDemo(1);
        }

        const std::string command = argv[1];
        if (command == "demo") {
            if (argc > 3) {
                printUsage(argv[0]);
                return 1;
            }
            const std::uint8_t bitsPerChannel = (argc == 3) ? parseBitsPerChannel(argv[2]) : 1;
            return runDemo(bitsPerChannel);
        }
        if (command == "embed-file") {
            if (argc < 5 || argc > 6) {
                printUsage(argv[0]);
                return 1;
            }
            const std::uint8_t bitsPerChannel = (argc == 6) ? parseBitsPerChannel(argv[5]) : 1;
            return runEmbedFile(argv[2], argv[3], argv[4], bitsPerChannel);
        }
        if (command == "extract-file") {
            if (argc < 4 || argc > 5) {
                printUsage(argv[0]);
                return 1;
            }
            const std::uint8_t bitsPerChannel = (argc == 5) ? parseBitsPerChannel(argv[4]) : 1;
            return runExtractFile(argv[2], argv[3], bitsPerChannel);
        }

        printUsage(argv[0]);
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
