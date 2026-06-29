#include "ImageDecoder.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Log.h"
#include "stb_image.h"

namespace LE::ImageUtils
{
bool DecodeBinaryPNG(std::span<const uint8> BinaryPNG, DecodedPNG& OutDecodedPNG)
{
    if (BinaryPNG.empty())
    {
        return false;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* rawPixelData = stbi_load_from_memory(BinaryPNG.data(), static_cast<int>(BinaryPNG.size()), &width, &height,
                                                        &channels, 4);

    if (!rawPixelData)
    {
        LE_ERROR("Failed to decode PNG");
        return false;
    }

    const size_t decodedImageSize = width * height * channels;
    OutDecodedPNG.Width = width;
    OutDecodedPNG.Height = height;
    OutDecodedPNG.Channels = channels;
    OutDecodedPNG.Data.resize(decodedImageSize);
    memcpy(OutDecodedPNG.Data.data(), rawPixelData, decodedImageSize);

    stbi_image_free(rawPixelData);

    return true;
}
}
