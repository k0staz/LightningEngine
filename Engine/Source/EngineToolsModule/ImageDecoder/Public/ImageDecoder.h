#pragma once
#include <span>

#include "CoreDefinitions.h"

namespace LE::ImageUtils
{
struct DecodedPNG
{
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Channels = 0;
    std::vector<uint8> Data;
};

bool DecodeBinaryPNG(std::span<const uint8> BinaryPNG, DecodedPNG& OutDecodedPNG);
}
