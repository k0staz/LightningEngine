#pragma once
#include <cstddef>
#include <memory>
#include <mutex>

#include "Templates/NonCopyable.h"
#include "tracy/Tracy.hpp"

namespace LE
{
class MemoryArena : public NonCopyable
{
public:
    explicit MemoryArena(size_t InChunkSize = 2 * 1024 * 1024)
        : ChunkSize(InChunkSize)
    {
    }

    ~MemoryArena() noexcept;

    MemoryArena(MemoryArena&& Other) noexcept;
    MemoryArena& operator=(MemoryArena&& Other) noexcept;

    template <typename T, typename... Args>
    T* Create(Args&&... InArgs)
    {
        std::lock_guard lock(Mutex);

        void* ptr = Allocate(sizeof(T), alignof(T));

        T* object = std::construct_at(static_cast<T*>(ptr), std::forward<Args>(InArgs)...);

        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            void* deleteNodePtr = Allocate(sizeof(DestroyNode), alignof(DestroyNode));
            DestroyHead = std::construct_at(static_cast<DestroyNode*>(deleteNodePtr), DestroyNode{
                                                .ObjectPtr = object,
                                                .DestroyFunc = [](void* ptr) noexcept
                                                {
                                                    std::destroy_at(static_cast<T*>(ptr));
                                                },
                                                .Next = DestroyHead
                                            });
        }

        return object;
    }

    void* Allocate(size_t Size, size_t Alignment) noexcept;

    void Reset() noexcept;

private:
    struct MemoryChunk
    {
        MemoryChunk* Next = nullptr;
        size_t Offset = 0;

        alignas(std::max_align_t) std::byte Data[];
    };

    struct DestroyNode
    {
        void* ObjectPtr = nullptr;
        void (*DestroyFunc)(void*) noexcept = nullptr;
        DestroyNode* Next = nullptr;
    };

    void MoveToNextChunk() noexcept;
    MemoryChunk* AllocateNewChunk() noexcept;

    std::mutex Mutex;

    MemoryChunk* ChunkHead = nullptr;
    MemoryChunk* CurrentChunk = nullptr;
    DestroyNode* DestroyHead = nullptr;
    size_t ChunkSize = 0;
};

template<typename T>
class ArenaAllocator
{
public:
    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::false_type;

    ArenaAllocator() = delete;
    explicit ArenaAllocator(MemoryArena& InArena) noexcept
        : Arena(&InArena)
    {}

    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& Other) noexcept
        : Arena(Other.Arena)
    {}

    [[nodiscard]] T* allocate(std::size_t Size)
    {
        if (Size == 0)
        {
            return nullptr;
        }

        if (Size > std::size_t(-1) / sizeof(T))
        {
            throw std::bad_array_new_length();
        }

        void* ptr = Arena->Allocate(Size * sizeof(T), alignof(T));
        if (!ptr)
        {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }

    void deallocate(T* Ptr, std::size_t N) noexcept
    {
    }

    template <typename U>
    bool operator==(const ArenaAllocator<U>& Other) const noexcept
    {
        return Arena == Other.Arena;
    }

    template <typename U>
    bool operator!=(const ArenaAllocator<U>& Other) const noexcept
    {
        return Arena != Other.Arena;
    }

private:
    template <typename U> friend class ArenaAllocator;

    MemoryArena* Arena;
};
}
