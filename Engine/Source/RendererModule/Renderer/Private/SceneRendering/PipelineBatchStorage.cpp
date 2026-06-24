#include "SceneRendering/PipelineBatchStorage.h"

namespace LE::Renderer
{
void PipelineBatchStorage::AddEntrance(const PermutationVariationKey& PermutationVariationKey, PipelineBatchData PipelineBatchData)
{
    size_t rowIndex = 0;
    if (PermutationToRowMap.contains(PermutationVariationKey))
    {
        rowIndex = PermutationToRowMap[PermutationVariationKey];
    }
    else
    {
        rowIndex = PermutationTypesCount++;
        PermutationToRowMap[PermutationVariationKey] = rowIndex;
        if (PipelineBatchDataStorage.size() <= rowIndex)
        {
            PipelineBatchDataStorage.resize(rowIndex + 1);
            PermutationVariationKeys.resize(rowIndex + 1);
        }
        PermutationVariationKeys[rowIndex] = PermutationVariationKey;
    }

    size_t columnIndex = 0;
    if (PermutationToColumnMap.contains(PermutationVariationKey))
    {
        columnIndex = PermutationToColumnMap[PermutationVariationKey];
    }
    else
    {
        columnIndex = PipelineBatchDataStorage[rowIndex].size();
        PermutationToColumnMap[PermutationVariationKey] = columnIndex;
        PipelineBatchDataStorage[rowIndex].emplace_back();
    }

    PipelineBatchDataStorage[rowIndex][columnIndex] = PipelineBatchData;
}

void PipelineBatchStorage::IncrementInstanceCount(const PermutationVariationKey& PermutationVariationKey)
{
    if (!Has(PermutationVariationKey))
    {
        return;
    }

    PipelineBatchDataStorage[PermutationToRowMap[PermutationVariationKey]][PermutationToColumnMap[PermutationVariationKey]].InstanceCount += 1;
}

std::vector<PipelineBatchStorage::PipelineBatchData>::const_iterator PipelineBatchStorage::GetPermutationVariationBegin(
    size_t PermutationIndex) const
{
    if (PermutationIndex >= PermutationTypesCount)
    {
        return PipelineBatchDataStorage[PermutationTypesCount].end();
    }

    return PipelineBatchDataStorage[PermutationIndex].begin();
}

std::vector<PipelineBatchStorage::PipelineBatchData>::const_iterator PipelineBatchStorage::GetPermutationVariationEnd(
    size_t PermutationIndex) const
{
    if (PermutationIndex >= PermutationTypesCount)
    {
        return PipelineBatchDataStorage[PermutationTypesCount].end();
    }

    return PipelineBatchDataStorage[PermutationIndex].end();
}

PermutationVariationKey PipelineBatchStorage::GetPermutationVariationKey(size_t PermutationIndex) const
{
    if (PermutationIndex >= PermutationTypesCount)
    {
        return PermutationVariationKey();
    }

    return PermutationVariationKeys[PermutationIndex];
}

void PipelineBatchStorage::Reset()
{
    PermutationToRowMap.clear();
    PermutationToColumnMap.clear();
    for (auto& PipelineRows : PipelineBatchDataStorage)
    {
        PipelineRows.clear();
    }
    PermutationTypesCount = 0;
}
}
