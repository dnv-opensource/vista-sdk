/**
 * @file Gmod.h
 * @brief Generic Product Model (Gmod) container and tree structure
 * @details Manages the complete Gmod hierarchy for a specific VIS version
 *          Provides node lookup, tree navigation, and iteration capabilities
 *          All GmodNode instances are owned and managed by this class
 */

#pragma once

#include "GmodNode.h"
#include "VisVersions.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dnv::vista::sdk
{
    struct GmodDto;

    /**
     * @class Gmod
     * @brief Generic Product Model container for a specific VIS version
     * @details The Gmod class manages the complete tree hierarchy of GmodNode instances
     *          for a specific VIS version. It provides:
     *          - Node storage and ownership
     *          - Fast node lookup by code with operator[] and node()
     *          - Root node access ("VE")
     *          - Static helper methods for node classification
     */
    class Gmod final
    {
        friend class VIS;

    private:
        /**
         * @brief Construct Gmod from DTO data
         * @param version The VIS version for this Gmod
         * @param dto DTO containing nodes and relations
         */
        explicit Gmod(VisVersion version, const GmodDto& dto);

    public:
        Gmod() = delete;
        Gmod(const Gmod&) = delete;
        Gmod(Gmod&&) noexcept = default;
        ~Gmod() = default;

        Gmod& operator=(const Gmod&) = delete;
        Gmod& operator=(Gmod&&) noexcept = delete;

        /**
         * @brief Access node by code with bounds checking
         * @param code Node code (e.g., "411.1", "VE", "C101")
         * @return Const reference to the GmodNode
         * @throws std::out_of_range if code not found
         */
        [[nodiscard]] const GmodNode& operator[](std::string_view code) const;

        /**
         * @brief Get the VIS version for this Gmod
         * @return The VisVersion enum value
         */
        [[nodiscard]] inline VisVersion version() const noexcept;

        /**
         * @brief Get the root node of the tree
         * @return Const reference to the root GmodNode (code "VE" - Vessel Equipment)
         */
        [[nodiscard]] inline const GmodNode& rootNode() const noexcept;

        /**
         * @brief Try to get a node by code
         * @param code Node code to search for (e.g., "411.1", "VE")
         * @return Optional pointer to the node if found, std::nullopt otherwise
         */
        [[nodiscard]] std::optional<const GmodNode*> node(std::string_view code) const noexcept;

        /**
         * @brief Get iterator to the first (code, node) pair
         * @details Both begin()/cbegin() return the same const_iterator type, since Gmod
         *          exposes read-only iteration. Iteration order is unspecified (hash map)
         * @return Iterator over (code, GmodNode) pairs, not in insertion order
         */
        [[nodiscard]] auto begin() const noexcept;

        /**
         * @brief Get iterator past the last (code, node) pair
         * @return Iterator to the element following the last element
         */
        [[nodiscard]] auto end() const noexcept;

        /** @brief Same as begin(), provided for explicit-const-iteration style */
        [[nodiscard]] auto cbegin() const noexcept;

        /** @brief Same as end(), provided for explicit-const-iteration style */
        [[nodiscard]] auto cend() const noexcept;

    private:
        const VisVersion m_visVersion;
        const GmodNode* m_rootNode;
        std::unordered_map<std::string, GmodNode> m_nodeMap;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/Gmod.inl"
