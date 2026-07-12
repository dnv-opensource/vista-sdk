#include "GmodVersioning.h"

#include "dnv/vista/sdk/core/VIS.h"

#include "SDK/core/internal/parsing/LocationSetsVisitor.h"

#include <stdexcept>

namespace dnv::vista::sdk::internal
{
    void GmodVersioning::addToPath(const Gmod& gmod, std::vector<GmodNode>& path, const GmodNode& node) const
    {
        // Fast path: if path is empty or last node is parent of new node
        if (path.empty() || path.back().isChild(node))
        {
            path.emplace_back(node);
            return;
        }

        // Slow path: need to find common ancestor and rebuild
        thread_local TraversalPath currentParentsBuf;
        thread_local TraversalPath remainingBuf;

        for (int j = static_cast<int>(path.size()) - 1; j >= 0; --j)
        {
            const GmodNode& parent = path[static_cast<size_t>(j)];
            const std::string_view parentCode = parent.code();

            const size_t currentParentsCount = static_cast<size_t>(j + 1);
            currentParentsBuf.resize(currentParentsCount);
            for (size_t k = 0; k < currentParentsCount; ++k)
            {
                currentParentsBuf[k] = &path[k];
            }

            remainingBuf.clear();
            const bool pathExists = gmod.pathExistsBetween(currentParentsBuf, node, remainingBuf);
            if (!pathExists)
            {
                bool hasOtherAssetFunction = false;
                for (size_t k = 0; k < currentParentsCount; ++k)
                {
                    const GmodNode* pathNode = currentParentsBuf[k];
                    if (pathNode->isAssetFunctionNode() && pathNode->code() != parentCode)
                    {
                        hasOtherAssetFunction = true;
                        break;
                    }
                }

                if (!hasOtherAssetFunction)
                {
                    throw std::runtime_error{ "Tried to remove last asset function node" };
                }

                const size_t removeIndex = static_cast<size_t>(j);
                for (size_t k = removeIndex + 1; k < path.size(); ++k)
                {
                    path[k - 1] = std::move(path[k]);
                }
                path.pop_back();
            }
            else
            {
                const auto& nodeLocation = node.location();

                path.reserve(path.size() + remainingBuf.size() + 1);

                if (nodeLocation.has_value())
                {
                    const auto& location = *nodeLocation;
                    for (const GmodNode* n : remainingBuf)
                    {
                        if (n->isIndividualizable(false, true))
                        {
                            path.emplace_back(n->withLocation(location));
                        }
                        else
                        {
                            path.emplace_back(*n);
                        }
                    }
                }
                else
                {
                    for (const GmodNode* n : remainingBuf)
                    {
                        path.emplace_back(*n);
                    }
                }

                path.emplace_back(node);
                return;
            }
        }

        path.emplace_back(node);
    }

    GmodVersioningNode::GmodVersioningNode(
        VisVersion version, const std::unordered_map<std::string, GmodNodeConversionDto>& dto)
        : visVersion{ version }
    {
        for (const auto& [code, conversionDto] : dto)
        {
            GmodNodeConversion conversion;

            conversion.source = conversionDto.source;

            conversion.target = conversionDto.target;
            conversion.oldAssignment = conversionDto.oldAssignment;
            conversion.newAssignment = conversionDto.newAssignment;
            conversion.deleteAssignment = conversionDto.deleteAssignment;

            versioningNodeChanges.emplace(code, std::move(conversion));
        }
    }

    std::optional<const GmodNodeConversion*> GmodVersioningNode::codeChanges(std::string_view code) const noexcept
    {
        auto it = versioningNodeChanges.find(std::string{ code });
        if (it != versioningNodeChanges.end())
        {
            return &it->second;
        }
        return std::nullopt;
    }

    GmodVersioning::GmodVersioning(const std::unordered_map<std::string, GmodVersioningDto>& dto)
    {
        for (const auto& [versionStr, versioningDto] : dto)
        {
            auto parsedVersion = VisVersions::fromString(versionStr);
            if (!parsedVersion.has_value())
            {
                throw std::invalid_argument{ "Invalid VIS version string: " + versionStr };
            }

            GmodVersioningNode versioningNode(*parsedVersion, versioningDto.items);
            m_versioningsMap.emplace(*parsedVersion, std::move(versioningNode));
        }
    }

    GmodVersioning::GmodVersioning(GmodVersioning&& other) noexcept
        : m_versioningsMap(std::move(other.m_versioningsMap))
    {}

    void GmodVersioning::validateSourceAndTargetVersions(VisVersion sourceVersion, VisVersion targetVersion) const
    {
        auto sourceStr = VisVersions::toString(sourceVersion);
        auto targetStr = VisVersions::toString(targetVersion);

        if (sourceStr.empty())
        {
            throw std::invalid_argument{ "Invalid source VIS Version: " + std::string{ sourceStr } };
        }

        if (targetStr.empty())
        {
            throw std::invalid_argument{ "Invalid target VISVersion: " + std::string{ targetStr } };
        }

        if (sourceVersion >= targetVersion)
        {
            throw std::invalid_argument{ "Source version must be less than target version" };
        }
    }

    void GmodVersioning::validateSourceAndTargetVersionPair(VisVersion sourceVersion, VisVersion targetVersion) const
    {
        if (sourceVersion >= targetVersion)
        {
            throw std::invalid_argument{ "Source version must be less than target version" };
        }

        if (static_cast<int>(targetVersion) - static_cast<int>(sourceVersion) != 1)
        {
            throw std::invalid_argument{ "Target version must be exactly one version higher than source version" };
        }
    }

    std::optional<const GmodVersioningNode*> GmodVersioning::versioningNode(VisVersion visVersion) const noexcept
    {
        auto it = m_versioningsMap.find(visVersion);
        if (it != m_versioningsMap.end())
        {
            return &it->second;
        }
        return std::nullopt;
    }

    std::optional<GmodNode> GmodVersioning::convertNode(
        VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion) const
    {
        validateSourceAndTargetVersions(sourceVersion, targetVersion);

        std::optional<GmodNode> node = sourceNode;
        VisVersion source = sourceVersion;

        while (static_cast<int>(source) < static_cast<int>(targetVersion))
        {
            if (!node.has_value())
            {
                break;
            }

            VisVersion target = static_cast<VisVersion>(static_cast<int>(source) + 1);

            node = convertNodeInternal(source, *node, target);
            source = static_cast<VisVersion>(static_cast<int>(source) + 1);
        }

        return node;
    }

    std::optional<GmodNode> GmodVersioning::convertNode(
        VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion, const Gmod& targetGmod) const
    {
        validateSourceAndTargetVersions(sourceVersion, targetVersion);

        if (sourceVersion == targetVersion)
        {
            return sourceNode;
        }

        std::optional<GmodNode> node = sourceNode;
        VisVersion source = sourceVersion;

        while (static_cast<int>(source) < static_cast<int>(targetVersion))
        {
            if (!node.has_value())
            {
                break;
            }

            VisVersion target = static_cast<VisVersion>(static_cast<int>(source) + 1);

            if (target == targetVersion)
            {
                node = convertNodeInternal(source, *node, target, targetGmod);
            }
            else
            {
                node = convertNodeInternal(source, *node, target);
            }

            source = target;
        }

        return node;
    }

    std::optional<GmodNode> GmodVersioning::convertNodeInternal(
        VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion) const
    {
        const auto& targetGmod = VIS::instance().gmod(targetVersion);
        return convertNodeInternal(sourceVersion, sourceNode, targetVersion, targetGmod);
    }

    std::optional<GmodNode> GmodVersioning::convertNodeInternal(
        VisVersion /*sourceVersion*/,
        const GmodNode& sourceNode,
        VisVersion targetVersion,
        const Gmod& targetGmod) const
    {
        validateSourceAndTargetVersionPair(sourceNode.version(), targetVersion);

        std::string_view nextCode = sourceNode.code();
        std::string nextCodeBuffer;

        auto versioningNodeOpt = versioningNode(targetVersion);
        if (versioningNodeOpt.has_value())
        {
            auto changeOpt = (*versioningNodeOpt)->codeChanges(sourceNode.code());
            if (changeOpt.has_value() && (*changeOpt)->target.has_value())
            {
                nextCodeBuffer = *(*changeOpt)->target;
                nextCode = nextCodeBuffer;
            }
        }

        auto targetNodeOpt = targetGmod.node(nextCode);
        if (!targetNodeOpt.has_value())
        {
            return std::nullopt;
        }

        const GmodNode* targetNode = *targetNodeOpt;

        // Fast path: no location, just copy the target node
        if (!sourceNode.location().has_value())
        {
            return *targetNode;
        }

        // Slow path: need to add location
        GmodNode result = targetNode->withLocation(*sourceNode.location());

        if (result.location() != sourceNode.location())
        {
            throw std::runtime_error{ "Failed to set location" };
        }

        return result;
    }

    std::optional<LocalIdBuilder> GmodVersioning::convertLocalId(
        const LocalIdBuilder& sourceLocalId, VisVersion targetVersion) const
    {
        auto sourceVersionOpt = sourceLocalId.version();
        if (!sourceVersionOpt.has_value())
        {
            throw std::invalid_argument{ "Cannot convert local ID without a specific VIS version" };
        }

        auto targetLocalId = LocalIdBuilder::create(targetVersion);

        auto primaryItemOpt = sourceLocalId.primaryItem();
        if (primaryItemOpt.has_value())
        {
            auto targetPrimaryItem = convertPath(*sourceVersionOpt, *primaryItemOpt, targetVersion);
            if (!targetPrimaryItem.has_value())
            {
                return std::nullopt;
            }

            targetLocalId = targetLocalId.withPrimaryItem(*targetPrimaryItem);
        }

        auto secondaryItemOpt = sourceLocalId.secondaryItem();
        if (secondaryItemOpt.has_value())
        {
            auto targetSecondaryItem = convertPath(*sourceVersionOpt, *secondaryItemOpt, targetVersion);
            if (!targetSecondaryItem.has_value())
            {
                return std::nullopt;
            }

            targetLocalId = targetLocalId.withSecondaryItem(*targetSecondaryItem);
        }

        targetLocalId = targetLocalId.withVerboseMode(sourceLocalId.isVerboseMode());

        if (sourceLocalId.quantity().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.quantity());
        }
        if (sourceLocalId.content().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.content());
        }
        if (sourceLocalId.calculation().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.calculation());
        }
        if (sourceLocalId.state().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.state());
        }
        if (sourceLocalId.command().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.command());
        }
        if (sourceLocalId.type().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.type());
        }
        if (sourceLocalId.position().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.position());
        }
        if (sourceLocalId.detail().has_value())
        {
            targetLocalId = targetLocalId.withMetadataTag(*sourceLocalId.detail());
        }

        return targetLocalId;
    }

    std::optional<LocalId> GmodVersioning::convertLocalId(const LocalId& sourceLocalId, VisVersion targetVersion) const
    {
        auto convertedBuilder = convertLocalId(sourceLocalId.builder(), targetVersion);
        if (!convertedBuilder.has_value())
        {
            return std::nullopt;
        }

        return convertedBuilder->build();
    }

    std::optional<GmodPath> GmodVersioning::convertPath(
        VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion) const
    {
        std::optional<GmodNode> targetEndNode = convertNode(sourceVersion, sourcePath.node(), targetVersion);
        if (!targetEndNode.has_value())
        {
            return std::nullopt;
        }

        auto& vis = VIS::instance();
        const auto& targetGmod = vis.gmod(targetVersion);

        if (targetEndNode->isRoot())
        {
            const auto rootNodeInGmodOpt = targetGmod.node(targetEndNode->code());
            if (!rootNodeInGmodOpt)
            {
                throw std::runtime_error{ "Failed to get root node from targetGmod during root path conversion" };
            }

            return GmodPath(std::vector<GmodNode>{}, **rootNodeInGmodOpt);
        }

        std::vector<std::pair<const GmodNode*, GmodNode>> qualifyingNodes;
        qualifyingNodes.reserve(sourcePath.length());

        for (const auto& [depth, node] : sourcePath.fullPath())
        {
            std::optional<GmodNode> convertedNodeOpt = convertNode(sourceVersion, node, targetVersion, targetGmod);
            if (!convertedNodeOpt.has_value())
            {
                throw std::runtime_error{ "Could not convert node forward" };
            }

            qualifyingNodes.emplace_back(&node, *std::move(convertedNodeOpt));
        }

        std::vector<GmodNode> potentialParents;
        potentialParents.reserve(qualifyingNodes.size() - 1);

        for (size_t i = 0; i < qualifyingNodes.size() - 1; ++i)
        {
            potentialParents.emplace_back(std::move(qualifyingNodes[i].second));
        }

        if (GmodPath::isValid(potentialParents, *targetEndNode))
        {
            // Fast path: already validated, skip re-verification
            std::vector<GmodNode> parentsVec(
                std::make_move_iterator(potentialParents.begin()), std::make_move_iterator(potentialParents.end()));

            return GmodPath{ std::move(parentsVec), std::move(*targetEndNode), true };
        }

        // Slow path: restore qualifyingNodes from potentialParents (move back)
        for (size_t i = 0; i < potentialParents.size(); ++i)
        {
            qualifyingNodes[i].second = std::move(potentialParents[i]);
        }

        std::vector<GmodNode> path;
        for (size_t i = 0; i < qualifyingNodes.size(); ++i)
        {
            const auto& qualifyingNode = qualifyingNodes[i];

            if (i > 0 && qualifyingNode.second.code() == qualifyingNodes[i - 1].second.code())
            {
                if (qualifyingNode.first->productType().has_value() &&
                    (!qualifyingNodes[i - 1].second.productType().has_value() ||
                     (*qualifyingNode.first->productType())->code() !=
                         (*qualifyingNodes[i - 1].second.productType())->code()))
                {
                    throw std::runtime_error{ "Failed to convert path at node " +
                                              std::string{ qualifyingNode.second.code() } +
                                              ". Uncovered case of merge where target node also has a new assignment" };
                }

                if (qualifyingNode.second.location().has_value())
                {
                    auto it = std::find_if(path.begin(), path.end(), [&](const GmodNode& n) {
                        return n.code() == qualifyingNode.second.code();
                    });

                    if (it != path.end())
                    {
                        size_t index = static_cast<size_t>(std::distance(path.begin(), it));

                        if (path[index].location().has_value() &&
                            path[index].location() != qualifyingNode.second.location())
                        {
                            throw std::runtime_error{
                                "Failed to convert path at node " + std::string{ qualifyingNode.second.code() } +
                                ". Uncovered case of multiple colliding locations while converting nodes"
                            };
                        }

                        if (!path[index].isIndividualizable(false, false))
                        {
                            throw std::runtime_error{ "Failed to convert path at node " +
                                                      std::string{ path[index].code() } +
                                                      ". Uncovered case of losing individualization information" };
                        }

                        if (!path[index].location().has_value())
                        {
                            path[index] = path[index].withLocation(qualifyingNode.second.location().value());
                        }
                    }
                }
                continue;
            }

            const bool codeChanged = qualifyingNode.first->code() != qualifyingNode.second.code();
            const std::optional<const GmodNode*> sourceNormalAssignment = qualifyingNode.first->productType();
            const std::optional<const GmodNode*> targetNormalAssignment = qualifyingNode.second.productType();
            const bool normalAssignmentChanged =
                sourceNormalAssignment.has_value() != targetNormalAssignment.has_value() ||
                (sourceNormalAssignment.has_value() && targetNormalAssignment.has_value() &&
                 (*sourceNormalAssignment)->code() != (*targetNormalAssignment)->code());
            // bool selectionChanged = false;

            if (codeChanged)
            {
                addToPath(targetGmod, path, qualifyingNode.second);
            }

            else if (normalAssignmentChanged)
            {
                // AC || AN || AD

                bool wasDeleted = sourceNormalAssignment.has_value() && !targetNormalAssignment.has_value();

                if (!codeChanged)
                {
                    addToPath(targetGmod, path, qualifyingNode.second);
                }

                if (wasDeleted)
                {
                    if (qualifyingNode.second.code() == targetEndNode->code())
                    {
                        if (i + 1 < qualifyingNodes.size())
                        {
                            const auto& next = qualifyingNodes[i + 1];
                            if (next.second.code() != qualifyingNode.second.code())
                            {
                                throw std::runtime_error{ "Normal assignment end node was deleted" };
                            }
                        }
                    }
                    continue;
                }
                else if (qualifyingNode.second.code() != targetEndNode->code())
                {
                    if (targetNormalAssignment.has_value())
                    {
                        addToPath(targetGmod, path, **targetNormalAssignment);

                        if (sourceNormalAssignment.has_value() && targetNormalAssignment.has_value() &&
                            (*sourceNormalAssignment)->code() != (*targetNormalAssignment)->code())
                        {
                            if (i + 1 < qualifyingNodes.size() &&
                                qualifyingNodes[i + 1].first->code() != (*sourceNormalAssignment)->code())
                            {
                                throw std::runtime_error{
                                    "Failed to convert path at node " + std::string{ qualifyingNode.second.code() } +
                                    ". Expected next qualifying source node to match target normal assignment"
                                };
                            }
                            ++i;
                        }
                    }
                }
            }

            // if (selectionChanged)
            // {
            //     // SC || SN || SD
            // }

            if (!codeChanged && !normalAssignmentChanged)
            {
                addToPath(targetGmod, path, qualifyingNode.second);
            }

            if (!path.empty() && path.back().code() == targetEndNode->code())
            {
                break;
            }
        }

        if (path.empty())
        {
            throw std::runtime_error{ "Path reconstruction resulted in an empty path" };
        }

        if (path.size() == 1)
        {
            return GmodPath(std::vector<GmodNode>{}, std::move(path[0]));
        }

        std::vector<GmodNode> potentialParentsFromPath;
        potentialParentsFromPath.reserve(path.size() - 1);

        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            potentialParentsFromPath.emplace_back(std::move(path[i]));
        }

        GmodNode targetEndNodeFromPath = std::move(path.back());

        LocationSetsVisitor visitor;
        for (size_t i = 0; i < potentialParentsFromPath.size() + 1; ++i)
        {
            const GmodNode& n =
                (i < potentialParentsFromPath.size()) ? potentialParentsFromPath[i] : targetEndNodeFromPath;
            auto set = visitor.visit(n, i, potentialParentsFromPath, targetEndNodeFromPath);
            if (!set.has_value())
            {
                if (n.location().has_value())
                {
                    break;
                }
                continue;
            }

            const auto& [start, end, location] = set.value();
            if (start == end)
            {
                continue;
            }

            for (size_t j = start; j <= end; ++j)
            {
                if (j < potentialParentsFromPath.size() && location.has_value())
                {
                    potentialParentsFromPath[j] = potentialParentsFromPath[j].withLocation(location.value());
                }
                else if (j >= potentialParentsFromPath.size() && location.has_value())
                {
                    targetEndNodeFromPath = targetEndNodeFromPath.withLocation(location.value());
                }
            }
        }

        if (!GmodPath::isValid(potentialParentsFromPath, targetEndNodeFromPath))
        {
            throw std::runtime_error{ "Did not end up with a valid path" };
        }

        std::vector<GmodNode> parentsVec(
            std::make_move_iterator(potentialParentsFromPath.begin()),
            std::make_move_iterator(potentialParentsFromPath.end()));
        return GmodPath{ std::move(parentsVec), targetEndNodeFromPath };
    }
} // namespace dnv::vista::sdk::internal
