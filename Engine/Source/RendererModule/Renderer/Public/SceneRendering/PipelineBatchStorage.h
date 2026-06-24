#pragma once
#include "RenderContributors/RenderContributerCore.h"
#include "Templates/NonCopyable.h"

namespace LE::Renderer
{
struct PermutationVariationKey
{
    RenderContributorTypeId MeshContributorTypeId = NullId{};
    RenderContributorId MeshInstanceId = NullId{};

    bool operator==(const PermutationVariationKey&) const = default;
};

class PipelineBatchStorage : public NonCopyable
{
public:
    struct PipelineBatchData
    {
        uint32 InstanceCount = 0;
        RenderContributorId MeshInstanceId = NullId{};
    };

    bool Has(const PermutationVariationKey& PermutationVariationKey) const
    {
        return PermutationToRowMap.contains(PermutationVariationKey) && PermutationToColumnMap.contains(PermutationVariationKey);
    }

    void AddEntrance(const PermutationVariationKey& PermutationVariationKey, PipelineBatchData PipelineBatchData);
    void IncrementInstanceCount(const PermutationVariationKey& PermutationVariationKey);

    size_t GetPermutationTypesCount() const { return PermutationTypesCount; }

    std::vector<PipelineBatchData>::const_iterator GetPermutationVariationBegin(size_t PermutationIndex) const;
    std::vector<PipelineBatchData>::const_iterator GetPermutationVariationEnd(size_t PermutationIndex) const;
    PermutationVariationKey GetPermutationVariationKey(size_t PermutationIndex) const;

    void Reset();

private:
    struct PermutationKeyHash
    {
        size_t operator()(const PermutationVariationKey& key) const noexcept
        {
            size_t seed = std::hash<RenderContributorId>{}(key.MeshInstanceId);

            return seed;
        }
    };

    struct PermutationVariationKeyHash
    {
        size_t operator()(const PermutationVariationKey& key) const noexcept
        {
            size_t seed = std::hash<RenderContributorId>{}(key.MeshInstanceId);

            auto hash_combine = [](size_t& s, size_t v)
            {
                s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
            };

            hash_combine(seed, std::hash<RenderContributorTypeId>{}(key.MeshContributorTypeId));
            return seed;
        }
    };

    std::unordered_map<PermutationVariationKey, uint32, PermutationKeyHash> PermutationToRowMap;
    std::unordered_map<PermutationVariationKey, uint32, PermutationVariationKeyHash> PermutationToColumnMap;
    size_t PermutationTypesCount = 0;
    std::vector<std::vector<PipelineBatchData>> PipelineBatchDataStorage;
    std::vector<PermutationVariationKey> PermutationVariationKeys;
};
}
