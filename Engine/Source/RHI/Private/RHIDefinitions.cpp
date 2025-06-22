#include "RHIDefinitions.h"

namespace LE::RHI
{
	PixelFormatInfo GPixelFormats[static_cast<uint8>(PixelFormat::Count)] =
	{
		PixelFormatInfo(PixelFormat::Invalid, 0),
		PixelFormatInfo(PixelFormat::R8G8B8A8_UNORM, 4),
		PixelFormatInfo(PixelFormat::R32G32B32A32_FLOAT, 16),
		PixelFormatInfo(PixelFormat::R32G32_FLOAT, 8),
	};
}
