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
    void* Allocate(size_t Size, size_t Alignment) noexcept;

    std::mutex Mutex;

    MemoryChunk* ChunkHead = nullptr;
    MemoryChunk* CurrentChunk = nullptr;
    DestroyNode* DestroyHead = nullptr;
    size_t ChunkSize = 0;
};
}
