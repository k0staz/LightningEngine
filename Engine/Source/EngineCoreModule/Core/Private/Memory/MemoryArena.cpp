#include "Memory/MemoryArena.h"

#include <memory>
#include <utility>

#include "CoreDefinitions.h"

namespace LE
{
MemoryArena::~MemoryArena() noexcept
{
    Reset();

    std::allocator<std::byte> alloc;
    MemoryChunk* current = ChunkHead;
    const size_t totalSize = ChunkSize + sizeof(MemoryChunk);
    while (current)
    {
        MemoryChunk* next = current->Next;
        alloc.deallocate((std::byte*)current, totalSize);
        current = next;
    }
}

MemoryArena::MemoryArena(MemoryArena&& Other) noexcept
    : ChunkHead(std::exchange(Other.ChunkHead, nullptr))
      , CurrentChunk(std::exchange(Other.CurrentChunk, nullptr))
      , DestroyHead(std::exchange(Other.DestroyHead, nullptr))
      , ChunkSize(Other.ChunkSize)
{
}

MemoryArena& MemoryArena::operator=(MemoryArena&& Other) noexcept
{
    if (this != &Other)
    {
        Reset();

        ChunkHead = std::exchange(Other.ChunkHead, nullptr);
        CurrentChunk = std::exchange(Other.CurrentChunk, nullptr);
        DestroyHead = std::exchange(Other.DestroyHead, nullptr);
        ChunkSize = Other.ChunkSize;
    }

    return *this;
}

void MemoryArena::Reset() noexcept
{
    std::lock_guard lock(Mutex);

    DestroyNode* current = DestroyHead;
    while (current)
    {
        current->DestroyFunc(current->ObjectPtr);
        current = current->Next;
    }
    DestroyHead = nullptr;

    MemoryChunk* currentChunk = ChunkHead;
    while (currentChunk)
    {
        currentChunk->Offset = 0;
        currentChunk = currentChunk->Next;
    }
    CurrentChunk = ChunkHead;
}

void MemoryArena::MoveToNextChunk() noexcept
{
    MemoryChunk* nextChunk = nullptr;
    if (!ChunkHead) [[unlikely]]
    {
        CurrentChunk = AllocateNewChunk();
        return;
    }

    if (!CurrentChunk->Next)
    {
        CurrentChunk->Next = AllocateNewChunk();
    }

    CurrentChunk = CurrentChunk->Next;
}

MemoryArena::MemoryChunk* MemoryArena::AllocateNewChunk() noexcept
{
    const size_t totalSize = ChunkSize + sizeof(MemoryChunk);

    std::allocator<std::byte> alloc;
    void* memory = alloc.allocate(totalSize);
    auto* newChunk = new (memory) MemoryChunk();

    newChunk->Next = nullptr;
    newChunk->Offset = 0;

    if (!ChunkHead) [[unlikely]]
    {
        ChunkHead = newChunk;
        CurrentChunk = newChunk;
    }

    return newChunk;
}

void* MemoryArena::Allocate(size_t Size, size_t Alignment) noexcept
{
    if (!CurrentChunk) [[unlikely]]
    {
        CurrentChunk = AllocateNewChunk();
    }

    while (true)
    {
        void* memory = CurrentChunk->Data + CurrentChunk->Offset;
        size_t remainingSize = ChunkSize - CurrentChunk->Offset;

        if (std::align(Alignment, Size, memory, remainingSize))
        {
            const size_t newOffset = ChunkSize - remainingSize + Size;
            CurrentChunk->Offset = newOffset;
            return memory;
        }

        MoveToNextChunk();
    }
}
}
