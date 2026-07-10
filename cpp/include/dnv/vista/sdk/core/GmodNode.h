/**
 * @file GmodNode.h
 * @brief Gmod node and metadata representation for VIS Generic Product Model
 * @details Represents nodes in the Gmod (Generic Product Model) tree hierarchy
 *            - GmodNodeMetadata: Descriptive information (category, type, names, definitions)
 *            - GmodNode: Tree node with code, location, metadata, and parent/child relationships
 */

#pragma once

#include "Locations.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dnv::vista::sdk
{
    enum class VisVersion : std::uint8_t;
    struct GmodNodeDto;

    /**
     * @class GmodNodeMetadata
     * @brief Metadata for a Gmod (Generic Product Model) node
     * @details Contains descriptive information about a Gmod node including:
     *          - Category: The node's category (e.g., "ASSET FUNCTION", "PRODUCT")
     *          - Type: The node's type (e.g., "COMPOSITION", "TYPE", "SELECTION")
     *          - Name: The node's name
     *          - CommonName: Optional common/friendly name
     *          - Definition: Optional technical definition
     *          - CommonDefinition: Optional user-friendly definition
     *          - InstallSubstructure: Optional flag for installation substructure
     *          - NormalAssignmentNames: Dictionary of normal assignment names
     *          - FullType: Computed property = "Category Type" (e.g., "ASSET FUNCTION COMPOSITION")
     *
     *          GmodNodeMetadata instances are created by GmodNode from GmodNodeDto data
     */
    class GmodNodeMetadata final
    {
        friend class GmodNode;

    private:
        /**
         * @brief Construct GmodNodeMetadata with all properties
         * @param category Node category (e.g., "ASSET FUNCTION", "PRODUCT")
         * @param type Node type (e.g., "COMPOSITION", "TYPE", "SELECTION")
         * @param name Node name
         * @param commonName Optional common/friendly name
         * @param definition Optional technical definition
         * @param commonDefinition Optional user-friendly definition
         * @param installSubstructure Optional installation substructure flag
         * @param normalAssignmentNames Dictionary of normal assignment names (can be empty)
         */
        explicit GmodNodeMetadata(
            std::string_view category,
            std::string_view type,
            std::string_view name,
            std::optional<std::string> commonName = std::nullopt,
            std::optional<std::string> definition = std::nullopt,
            std::optional<std::string> commonDefinition = std::nullopt,
            std::optional<bool> installSubstructure = std::nullopt,
            std::unordered_map<std::string, std::string> normalAssignmentNames = {}) noexcept;

    public:
        GmodNodeMetadata() = delete;
        GmodNodeMetadata(const GmodNodeMetadata&) = default;
        GmodNodeMetadata(GmodNodeMetadata&&) noexcept = default;
        ~GmodNodeMetadata() = default;

        GmodNodeMetadata& operator=(const GmodNodeMetadata&) = default;
        GmodNodeMetadata& operator=(GmodNodeMetadata&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const GmodNodeMetadata& other) const noexcept;

        /**
         * @brief Get the node category
         * @return Category string view (e.g., "ASSET FUNCTION", "PRODUCT")
         */
        [[nodiscard]] inline std::string_view category() const noexcept;

        /**
         * @brief Get the node type
         * @return Type string view (e.g., "COMPOSITION", "TYPE", "SELECTION")
         */
        [[nodiscard]] inline std::string_view type() const noexcept;

        /**
         * @brief Get the full type (computed property)
         * @return Full type as "Category Type" (e.g., "ASSET FUNCTION COMPOSITION")
         */
        [[nodiscard]] inline std::string_view fullType() const noexcept;

        /** @brief Get the node name */
        [[nodiscard]] inline std::string_view name() const noexcept;

        /** @brief Get the common/friendly name (nullopt if not set) */
        [[nodiscard]] inline const std::optional<std::string>& commonName() const noexcept;

        /** @brief Get the technical definition (nullopt if not set) */
        [[nodiscard]] inline const std::optional<std::string>& definition() const noexcept;

        /** @brief Get the user-friendly definition (nullopt if not set) */
        [[nodiscard]] inline const std::optional<std::string>& commonDefinition() const noexcept;

        /**
         * @brief Get the optional installation substructure flag
         * @return Optional bool indicating if node has installation substructure
         */
        [[nodiscard]] inline std::optional<bool> installSubstructure() const noexcept;

        /**
         * @brief Get the normal assignment names dictionary
         * @return Read-only reference to normal assignment names map
         */
        [[nodiscard]] inline const std::unordered_map<std::string, std::string>& normalAssignmentNames() const noexcept;

    private:
        std::string m_category;
        std::string m_type;
        std::string m_name;
        std::optional<std::string> m_commonName;
        std::optional<std::string> m_definition;
        std::optional<std::string> m_commonDefinition;
        std::optional<bool> m_installSubstructure;
        std::unordered_map<std::string, std::string> m_normalAssignmentNames;
        std::string m_fullType;
    };

    /**
     * @class GmodNode
     * @brief Represents a node in the Gmod (Generic Product Model) tree hierarchy
     * @details A GmodNode is a tree structure element containing:
     *          - Code: Unique identifier (e.g., "VE" for root, "411.1" for specific nodes)
     *          - Location: Optional VIS location
     *          - Metadata: Descriptive information (category, type, names, definitions)
     *          - Children/Parents: Navigation pointers (managed by Gmod class)
     */
    class GmodNode final
    {
    private:
        /**
         * @brief Internal constructor - creates GmodNode from DTO
         * @param version The VIS version this node belongs to
         * @param dto DTO containing node data
         * @throws std::bad_alloc if metadata allocation fails
         */
        explicit GmodNode(VisVersion version, const GmodNodeDto& dto);

    public:
        GmodNode() = delete;
        GmodNode(const GmodNode&) = default;
        GmodNode(GmodNode&&) noexcept = default;
        ~GmodNode() = default;

        GmodNode& operator=(const GmodNode&) = default;
        GmodNode& operator=(GmodNode&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const GmodNode& other) const noexcept;

        /**
         * @brief Get the VIS version for this node
         * @return The VisVersion enum value
         */
        inline VisVersion version() const noexcept;

        /**
         * @brief Get the node code
         * @return Node code as string view (e.g., "VE", "411.1")
         */
        inline std::string_view code() const noexcept;

        /**
         * @brief Get the optional location
         * @return Optional Location reference
         */
        inline const std::optional<Location>& location() const noexcept;

        /**
         * @brief Get the node metadata
         * @return Reference to GmodNodeMetadata
         */
        inline const GmodNodeMetadata& metadata() const noexcept;

        /**
         * @brief Get child nodes
         * @return Vector of child node pointers
         */
        inline const std::vector<GmodNode*>& children() const noexcept;

        /**
         * @brief Get parent nodes
         * @return Vector of parent node pointers
         */
        inline const std::vector<GmodNode*>& parents() const noexcept;

        /**
         * @brief Get product type child if this is a function node
         * @return Optional pointer to product type child node, or nullopt if not applicable
         * @note Returns a non-owning pointer to the child node managed by Gmod
         */
        inline std::optional<const GmodNode*> productType() const noexcept;

        /**
         * @brief Get product selection child if this is a function node
         * @return Optional pointer to product selection child node, or nullopt if not applicable
         * @note Returns a non-owning pointer to the child node managed by Gmod
         */
        inline std::optional<const GmodNode*> productSelection() const noexcept;

        /**
         * @brief Check if this is a function composition node
         * @return True if category is ASSET/PRODUCT FUNCTION and type is COMPOSITION
         */
        inline bool isFunctionComposition() const noexcept;

        /**
         * @brief Check if this node is mappable
         * @return True if the node can be used in mapping
         */
        inline bool isMappable() const noexcept;

        /**
         * @brief Check if this is a product selection node
         * @return True if category is PRODUCT and type is SELECTION
         */
        inline bool isProductSelection() const noexcept;

        /**
         * @brief Check if this is a product type node
         * @return True if category is PRODUCT and type is TYPE
         */
        inline bool isProductType() const noexcept;

        /**
         * @brief Check if this is an asset node
         * @return True if category is ASSET
         */
        inline bool isAsset() const noexcept;

        /**
         * @brief Check if this is a leaf node
         * @return True if this is an ASSET FUNCTION LEAF or PRODUCT FUNCTION LEAF
         */
        inline bool isLeafNode() const noexcept;

        /**
         * @brief Check if this is a function node
         * @return True if category is not PRODUCT or ASSET
         */
        inline bool isFunctionNode() const noexcept;

        /**
         * @brief Check if this is an asset function node
         * @return True if category is ASSET FUNCTION
         */
        inline bool isAssetFunctionNode() const noexcept;

        /**
         * @brief Check if this is the root node
         * @return True if code is "VE"
         */
        inline bool isRoot() const noexcept;

        /**
         * @brief Check if a node is a child of this node
         * @param node Node to check
         * @return True if the node is a child
         */
        inline bool isChild(const GmodNode& node) const noexcept;

        /**
         * @brief Check if a node code is a child of this node
         * @param code Node code to check
         * @return True if a child has this code
         */
        inline bool isChild(std::string_view code) const noexcept;

        /**
         * @brief Get string representation of this node
         * @return String in format "Code" or "Code-Location"
         */
        inline std::string toString() const noexcept;

        /**
         * @brief Append string representation to an existing string
         * @param out String to append to
         */
        inline void toString(std::string& out) const noexcept;

    private:
        /**
         * @brief Create a copy of this node with a different location
         * @param location The new location
         * @return Copy of this node with the specified location
         */
        inline GmodNode withLocation(const Location& location) const noexcept;

        /**
         * @brief Create a copy of this node with a location parsed from string
         * @param locationStr Location string to parse
         * @return Optional GmodNode with parsed location, nullopt if parsing fails
         */
        std::optional<GmodNode> withLocation(std::string_view locationStr) const;

        /**
         * @brief Create a copy of this node with a location parsed from string with error details
         * @param locationStr Location string to parse
         * @param errors Output ParsingErrors containing detailed error information if parsing fails
         * @return Optional GmodNode with parsed location, nullopt if parsing fails
         */
        std::optional<GmodNode> withLocation(std::string_view locationStr, ParsingErrors& errors) const;

        /**
         * @brief Create a copy of this node without location
         * @return Copy of this node with location removed
         */
        inline GmodNode withoutLocation() const;

        /**
         * @brief Check if this node can be individualized
         * @param isTargetNode True if this is the target node in the path
         * @param isInSet True if this node is in an individualizable set
         * @return True if the node can be individualized
         */
        inline bool isIndividualizable(bool isTargetNode = false, bool isInSet = false) const noexcept;

        /**
         * @brief Add a child node to this node's children list
         * @param child Pointer to the child node to add
         * @note Only called by Gmod class during tree construction
         */
        void addChild(GmodNode* child) noexcept;

        /**
         * @brief Add a parent node to this node's parents list
         * @param parent Pointer to the parent node to add
         * @note Only called by Gmod class during tree construction
         */
        void addParent(GmodNode* parent) noexcept;

        /**
         * @brief Optimize memory usage and build child lookup set
         * @details Shrinks vectors to fit and creates the m_childrenSet for fast lookups
         * @note Only called by Gmod class after tree construction is complete
         */
        void trim() noexcept;

        /**
         * @brief Set location in-place
         * @param location The new location
         * @note Only for use by friend parsing functions to avoid copies
         */
        inline void setLocation(const Location& location) noexcept;

        /**
         * @brief Set location in-place with move semantics
         * @param location The new location
         * @note Only for use by friend parsing functions to avoid copies
         */
        inline void setLocation(Location&& location) noexcept;

    private:
        VisVersion m_visVersion;
        std::string m_code;
        std::optional<Location> m_location;
        std::shared_ptr<const GmodNodeMetadata> m_metadata;
        std::vector<GmodNode*> m_children;
        std::vector<GmodNode*> m_parents;
        std::shared_ptr<std::unordered_set<std::string>> m_childrenSet;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/GmodNode.inl"
