/**
 * @file GmodPath.h
 * @brief Validated path through a Gmod tree with individualization and traversal support
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"
#include "GmodNode.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dnv::vista::sdk
{
    class Gmod;
    class GmodIndividualizableSet;
    class Location;
    class Locations;

    enum class VisVersion : std::uint8_t;

    namespace internal
    {
        class GmodVersioning;
    } // namespace internal

    /** @brief Validated path through a Gmod tree, consisting of parent nodes leading to a target node */
    class GmodPath final
    {
    private:
        friend class GmodIndividualizableSet;
        friend class internal::GmodVersioning;
        friend TraversalHandlerResult internal::parseHandler(
            internal::ParseContext&, const TraversalPath&, const GmodNode&);
        friend internal::GmodParsePathResult internal::fromFullPath(
            std::string_view, const Gmod&, const Locations&) noexcept;

    private:
        /**
         * @brief Internal constructor with optional validation skip
         * @param skipVerify If true, skips validation checks (used internally for performance)
         */
        GmodPath(std::vector<GmodNode>&& parents, GmodNode node, bool skipVerify);

        /**
         * @brief Internal non-const indexer for modification by GmodIndividualizableSet
         * @param index Depth in the path (0 = root, length-1 = node)
         * @return Mutable reference to the node at the specified depth
         */
        inline GmodNode& operator[](size_t index);

    public:
        /**
         * @brief Constructs a validated Gmod path
         * @param parents Parent nodes in the path hierarchy (moved)
         * @param node Target node at the end of the path
         * @throws std::invalid_argument If path validation fails
         */
        explicit GmodPath(std::vector<GmodNode>&& parents, GmodNode node);

        GmodPath() = delete;
        GmodPath(const GmodPath&) = default;
        GmodPath(GmodPath&&) noexcept = default;
        ~GmodPath() = default;

        GmodPath& operator=(const GmodPath&) = default;
        GmodPath& operator=(GmodPath&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const GmodPath& other) const noexcept;

        /**
         * @brief Access node at specified depth
         * @param index Depth in the path (0 = root, length-1 = node)
         * @return Const reference to the node at the specified depth
         */
        inline const GmodNode& operator[](size_t index) const;

        /** @brief Get the VIS version for this path */
        inline VisVersion version() const noexcept;

        /** @brief Get the target node at the end of the path */
        inline const GmodNode& node() const noexcept;

        /** @brief Get the parent nodes in the path hierarchy */
        inline const std::vector<GmodNode>& parents() const noexcept;

        /** @brief Get the total number of nodes in the path (parents + node) */
        inline size_t length() const noexcept;

        /** @brief Get all individualizable sets in this path */
        std::vector<GmodIndividualizableSet> individualizableSets() const;

        /**
         * @brief Get the normal assignment name for a node at specified depth
         * @param nodeDepth Depth of the node to query (0-based)
         * @return Normal assignment name if found, nullopt otherwise
         */
        std::optional<std::string> normalAssignmentName(size_t nodeDepth) const;

        /**
         * @brief Get common names for all function nodes in the path
         * @return Vector of (depth, name) pairs for each function node
         */
        std::vector<std::pair<size_t, std::string>> commonNames() const;

        /**
         * @brief Validate a parent-child node sequence
         * @param parents Sequence of parent nodes
         * @param node Target node to validate
         * @return True if the sequence forms a valid Gmod path
         */
        static inline bool isValid(std::span<const GmodNode> parents, const GmodNode& node);

        /** @brief Check if the target node is mappable to equipment */
        inline bool isMappable() const noexcept;

        /** @brief Check if the path contains any individualizable nodes */
        [[nodiscard]] bool isIndividualizable() const;

        /** @brief Create a copy of this path with all locations removed */
        [[nodiscard]] GmodPath withoutLocations() const;

        /** @brief Convert path to short string representation (leaf nodes only, separated by '/') */
        inline std::string toString() const;

        /**
         * @brief Append short string representation to an existing string
         * @param builder Builder to append the path to
         * @param separator Character to use between nodes (default '/')
         */
        inline void toString(StringBuilder& builder, char separator = '/') const;

        /** @brief Convert path to full string representation (all nodes, separated by '/') */
        std::string toFullPathString() const;

        /** @brief Append full string representation to an existing string */
        inline void toFullPathString(StringBuilder& builder) const;

        /** @brief Convert path to diagnostic dump string (codes, names, common names, normal assignments) */
        inline std::string toStringDump() const;

        /**
         * @brief Append diagnostic dump to an existing string
         * @param builder Builder to append the dump to
         */
        inline void toStringDump(StringBuilder& builder) const;

        /** @brief Range type for depth-indexed full path traversal - yields (depth, node) pairs */
        class FullPathRange final
        {
        public:
            struct Iterator
            {
                using iterator_category = std::forward_iterator_tag;
                using value_type = std::pair<size_t, const GmodNode&>;
                using difference_type = std::ptrdiff_t;
                using pointer = const value_type*;
                using reference = value_type;

                explicit Iterator(const GmodPath* path, size_t depth)
                    : m_path{ path },
                      m_depth{ depth }
                {}

                reference operator*() const { return { m_depth, (*m_path)[m_depth] }; }

                Iterator& operator++()
                {
                    ++m_depth;
                    return *this;
                }

                Iterator operator++(int)
                {
                    Iterator tmp = *this;
                    ++m_depth;
                    return tmp;
                }

                [[nodiscard]] bool operator==(const Iterator& other) const noexcept { return m_depth == other.m_depth; }

            private:
                const GmodPath* m_path;
                size_t m_depth;
            };

            explicit FullPathRange(const GmodPath* path, size_t start = 0)
                : m_path{ path },
                  m_start{ start }
            {}

            /** @brief Iterator starting at depth `start` (not necessarily the path root) */
            [[nodiscard]] Iterator begin() const { return Iterator(m_path, m_start); }

            /** @brief Iterator one past the last (depth, node) pair in the path */
            [[nodiscard]] Iterator end() const { return Iterator(m_path, m_path->length()); }

        private:
            const GmodPath* m_path;
            size_t m_start;
        };

        /**
         * @brief Get a range for full path traversal
         * @return Range supporting range-based for loops yielding (depth, node) pairs
         * @code
         * for (auto [depth, node] : path.fullPath()) { ... }
         * @endcode
         */
        inline FullPathRange fullPath() const;

        /**
         * @brief Get a range for full path traversal starting from a specific depth
         * @param fromDepth Starting depth (0-based)
         * @return Range supporting range-based for loops
         */
        inline FullPathRange fullPathFrom(size_t fromDepth) const;

        /**
         * @brief Parse a short path string into a GmodPath
         * @param item Short path string (e.g., "411.1/C101.72/I101")
         * @param visVersion VIS version to use for Gmod and Locations lookup
         * @return Parsed GmodPath, or nullopt if parsing failed
         */
        [[nodiscard]] static std::optional<GmodPath> fromShortPath(
            std::string_view item, VisVersion visVersion) noexcept;

        /**
         * @brief Parse a short path string into a GmodPath
         * @param item Short path string (e.g., "411.1/C101.72/I101")
         * @param gmod Gmod instance to resolve nodes from
         * @param locations Locations instance to parse location suffixes
         * @return Parsed GmodPath, or nullopt if parsing failed
         */
        [[nodiscard]] static std::optional<GmodPath> fromShortPath(
            std::string_view item, const Gmod& gmod, const Locations& locations) noexcept;

        /**
         * @brief Parse a short path string into a GmodPath with error reporting
         * @param item Short path string (e.g., "411.1/C101.72/I101")
         * @param gmod Gmod instance to resolve nodes from
         * @param locations Locations instance to parse location suffixes
         * @param errors Output parameter populated with parsing errors on failure
         * @return Parsed GmodPath, or nullopt if parsing failed
         */
        [[nodiscard]] static std::optional<GmodPath> fromShortPath(
            std::string_view item, const Gmod& gmod, const Locations& locations, ParsingErrors& errors) noexcept;

        /**
         * @brief Parse a full path string into a GmodPath
         * @param fullPathStr Full path string starting with root (e.g., "VE/400a/410/411/411i/411.1")
         * @param gmod Gmod instance to resolve nodes from
         * @param locations Locations instance to parse location suffixes
         * @return Parsed GmodPath, or nullopt if parsing failed
         */
        [[nodiscard]] static std::optional<GmodPath> fromFullPath(
            std::string_view fullPathStr, const Gmod& gmod, const Locations& locations) noexcept;

        /**
         * @brief Parse a full path string into a GmodPath with error reporting
         * @param fullPathStr Full path string starting with root
         * @param gmod Gmod instance to resolve nodes from
         * @param locations Locations instance to parse location suffixes
         * @param errors Output parameter populated with parsing errors on failure
         * @return Parsed GmodPath, or nullopt if parsing failed
         */
        [[nodiscard]] static std::optional<GmodPath> fromFullPath(
            std::string_view fullPathStr, const Gmod& gmod, const Locations& locations, ParsingErrors& errors) noexcept;

    private:
        VisVersion m_visVersion;
        std::vector<GmodNode> m_parents;
        GmodNode m_node;
    };

    /** @brief Contiguous sequence of nodes in a GmodPath that can be individualized with a location */
    class GmodIndividualizableSet final
    {
    public:
        /**
         * @brief Constructs an individualizable set from node indices
         * @param nodeIndices Indices of nodes in the path that form this set
         * @param sourcePath Source path containing the nodes (will be copied)
         * @throws std::runtime_error If set validation fails
         */
        explicit GmodIndividualizableSet(const std::vector<int>& nodeIndices, const GmodPath& sourcePath);

        GmodIndividualizableSet() = delete;
        GmodIndividualizableSet(const GmodIndividualizableSet&) = default;
        GmodIndividualizableSet(GmodIndividualizableSet&&) noexcept = default;
        ~GmodIndividualizableSet() = default;

        GmodIndividualizableSet& operator=(const GmodIndividualizableSet&) = delete;
        GmodIndividualizableSet& operator=(GmodIndividualizableSet&&) noexcept = delete;

        /**
         * @brief Build and return the modified path (can only be called once)
         * @return GmodPath with modified locations
         * @throws std::runtime_error If already built
         */
        GmodPath build();

        /**
         * @brief Get the nodes in this individualizable set
         * @return Vector of node copies
         * @throws std::runtime_error If already built
         */
        std::vector<GmodNode> nodes() const;

        /** @brief Get the indices of nodes in this set */
        inline const std::vector<int>& nodeIndices() const noexcept;

        /**
         * @brief Get the current location assigned to this set
         * @return Location if set, nullopt otherwise
         * @throws std::runtime_error If already built
         */
        inline std::optional<Location> location() const;

        /**
         * @brief Convert set to string representation (leaf nodes separated by '/')
         * @throws std::runtime_error If already built
         */
        [[nodiscard]] std::string toString() const;

    private:
        std::vector<int> m_nodeIndices;
        std::optional<GmodPath> m_path;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/GmodPath.inl"
