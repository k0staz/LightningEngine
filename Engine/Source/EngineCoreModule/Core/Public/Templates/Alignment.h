#pragma once
#include "CoreDefinitions.h"
#include "CoreConcepts.h"

namespace LE
{
template <Alignable T>
constexpr T Align(T Value, uint64 Alignment)
{
    if constexpr (std::is_pointer_v<T>)
    {
        std::uintptr_t intValue = reinterpret_cast<std::uintptr_t>(Value);
        uint64 aligned = (intValue + Alignment - 1) & ~(Alignment - 1);
        return reinterpret_cast<T>(aligned);
    }
    else
    {
        uint64 intValue = static_cast<uint64>(Value);
        uint64 aligned = (intValue + Alignment - 1) & ~(Alignment - 1);
        return static_cast<T>(aligned);
    }
}

template <Alignable T>
constexpr T AlignDown(T Value, uint64 Alignment)
{
    if constexpr (std::is_pointer_v<T>)
    {
        std::uintptr_t intValue = reinterpret_cast<std::uintptr_t>(Value);
        uint64 aligned = intValue & ~(Alignment - 1);
        return reinterpret_cast<T>(aligned);
    }
    else
    {
        uint64 intValue = static_cast<uint64>(Value);
        uint64 aligned = intValue & ~(Alignment - 1);
        return static_cast<T>(aligned);
    }
}

template <Alignable T>
constexpr bool IsAligned(T Value, uint64 Alignment)
{
    if constexpr (std::is_pointer_v<T>)
    {
        std::uintptr_t intValue = reinterpret_cast<std::uintptr_t>(Value);
        return !(intValue & (Alignment - 1));
    }
    else
    {
        uint64 intValue = static_cast<uint64>(Value);
        return !(intValue & (Alignment - 1));
    }
}

/**
 * @brief Automatically pads and aligns any type T up to a specific ByteAlignment boundary.
 */
template <typename T, size_t ByteAlignment = 16>
struct __declspec(empty_bases) alignas(ByteAlignment) TStructAligned
{
    TStructAligned() = default;

    TStructAligned(T InValue)
        : Value(InValue)
    {
    }

    T Value;

private:
    static constexpr size_t RawSize = sizeof(T);
    static constexpr size_t Remainder = RawSize % ByteAlignment;
    static constexpr size_t PaddingSize = (Remainder == 0) ? 0 : (ByteAlignment - Remainder);

    struct EmptyPadding
    {
    };

    using PaddingType = std::conditional_t<
        PaddingSize == 0,
        EmptyPadding,
        std::array<uint8_t, PaddingSize>
    >;

    [[msvc::no_unique_address]][[no_unique_address]] PaddingType Padding = {};

public:
    static constexpr size_t Size = RawSize + PaddingSize;
};
}
