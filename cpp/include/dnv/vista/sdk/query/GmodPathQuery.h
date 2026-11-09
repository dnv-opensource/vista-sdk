/**
 * @file GmodPathQuery.h
 * @brief Query API for matching GmodPath instances
 * @details Provides fluent API for building queries to filter GmodPaths based on nodes and locations
 *          Supports path-based queries (starting from existing path) and node-based queries (from scratch)
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "dnv/vista/sdk/core/GmodPath.h"
#include "dnv/vista/sdk/core/Locations.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dnv::vista::sdk
{
    class GmodPathQueryBuilder;

    /**
     * @class GmodPathQuery
     * @brief Immutable query for matching GmodPath instances
     * @details Created via GmodPathQueryBuilder. Provides match() method to test GmodPaths
     *
     * @par Example:
     * @code
     * auto basePath = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations).value();
     * auto query = GmodPathQueryBuilder::from(basePath)
     *     .withoutLocations()
     *     .build();
     *
     * bool matches = query.match(otherPath);
     * @endcode
     */
    class GmodPathQuery final
    {
        friend class GmodPathQueryBuilder;
        friend class LocalIdQuery;
        friend class LocalIdQueryBuilder;

    private:
        /**
         * @brief Construct from a builder
         * @param builder Builder whose state is copied into this query
         */
        explicit GmodPathQuery(const GmodPathQueryBuilder* builder);

    public:
        DNV_VISTA_SDK_CPP_API GmodPathQuery(const GmodPathQuery& other);
        DNV_VISTA_SDK_CPP_API GmodPathQuery(GmodPathQuery&&) noexcept;
        ~GmodPathQuery() = default;

        DNV_VISTA_SDK_CPP_API GmodPathQuery& operator=(const GmodPathQuery& other);
        DNV_VISTA_SDK_CPP_API GmodPathQuery& operator=(GmodPathQuery&&) noexcept;

        /**
         * @brief Match against a GmodPath
         * @param other The GmodPath to test
         * @return true if other matches the query criteria
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool match(const GmodPath& other) const;

    private:
        /**
         * @brief Get the builder that created this query
         * @return Pointer to builder
         */
        [[nodiscard]] const GmodPathQueryBuilder* builder() const noexcept;

        std::unique_ptr<GmodPathQueryBuilder> m_builder;
    };

    /**
     * @class GmodPathQueryBuilder
     * @brief Abstract base class for building GmodPath queries
     * @details Two concrete implementations:
     *          - Path: Start from existing GmodPath, modify specific nodes
     *          - Nodes: Build from scratch, specify individual nodes
     */
    class GmodPathQueryBuilder
    {
        friend class GmodPathQuery;

    private:
        /**
         * @struct NodeItem
         * @brief Internal filter item for node matching
         */
        struct NodeItem
        {
            GmodNode node;
            std::vector<Location> locations;
            bool matchAllLocations = false;
            bool ignoreInMatching = false;

            NodeItem() = delete;
            NodeItem(const GmodNode& n, const std::vector<Location>& locs);
        };

    public:
        class Path;
        class Nodes;

    private:
        GmodPathQueryBuilder() = default;

    public:
        DNV_VISTA_SDK_CPP_API GmodPathQueryBuilder(const GmodPathQueryBuilder&);
        DNV_VISTA_SDK_CPP_API GmodPathQueryBuilder(GmodPathQueryBuilder&&) noexcept;
        DNV_VISTA_SDK_CPP_API virtual ~GmodPathQueryBuilder();

        DNV_VISTA_SDK_CPP_API GmodPathQueryBuilder& operator=(const GmodPathQueryBuilder&);
        DNV_VISTA_SDK_CPP_API GmodPathQueryBuilder& operator=(GmodPathQueryBuilder&&) noexcept;

        /**
         * @brief Create empty Nodes builder
         * @return New Nodes builder
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API Nodes create();

        /**
         * @brief Create Path builder from existing GmodPath
         * @param path GmodPath to use as base
         * @return New Path builder
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API Path from(const GmodPath& path);

        /**
         * @brief Build immutable query
         * @return GmodPathQuery instance
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API GmodPathQuery build() const;

        /**
         * @brief Internal match implementation
         * @param other GmodPath to test (nullptr returns false)
         * @return true if path matches the query criteria
         */
        [[nodiscard]] virtual bool match(const GmodPath* other) const = 0;

    protected:
        [[nodiscard]] virtual std::unique_ptr<GmodPathQueryBuilder> clone() const = 0;

        /**
         * @brief Common matching logic for Path and Nodes queries
         * @param filter Map of node codes to filter items
         * @param target The target GmodPath to match against (already converted to latest version)
         * @param checkIgnoreFlag Whether to check the ignoreInMatching flag
         * @return true if target matches all filter criteria
         */
        [[nodiscard]] static bool matchFilterAgainstTarget(
            const std::unordered_map<std::string, NodeItem, StringHash, std::equal_to<>>& filter,
            const GmodPath& target,
            bool checkIgnoreFlag);

        std::unordered_map<std::string, NodeItem, StringHash, std::equal_to<>> m_filter;
    };

    /**
     * @class GmodPathQueryBuilder::Path
     * @brief Path-based query builder
     */
    class GmodPathQueryBuilder::Path final : public GmodPathQueryBuilder
    {
    public:
        Path() = delete;
        DNV_VISTA_SDK_CPP_API explicit Path(const GmodPath& path);
        DNV_VISTA_SDK_CPP_API Path(const Path&);
        DNV_VISTA_SDK_CPP_API Path(Path&&) noexcept;
        DNV_VISTA_SDK_CPP_API virtual ~Path() override;

        DNV_VISTA_SDK_CPP_API Path& operator=(const Path&);
        DNV_VISTA_SDK_CPP_API Path& operator=(Path&&) noexcept;

        /**
         * @brief Get the base GmodPath
         * @return Reference to the GmodPath
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const GmodPath& path() const noexcept;

        /**
         * @brief Configure node to match all locations
         * @param select Function to select node from set nodes dictionary
         * @param matchAllLocations If true, match any location individualization
         * @return New Path builder with node configured
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withNode(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select,
            bool matchAllLocations = false) const&;

        /**
         * @brief Configure node to match all locations
         * @param select Function to select node from set nodes dictionary
         * @param matchAllLocations If true, match any location individualization
         * @return New Path builder with node configured
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withNode(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select,
            bool matchAllLocations = false) &&;

        /**
         * @brief Configure node with specific locations
         * @param select Function to select node from set nodes dictionary
         * @param locations Locations to match (empty = no locations)
         * @return New Path builder with node configured
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withNode(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select,
            const std::vector<Location>& locations) const&;

        /**
         * @brief Configure node with specific locations
         * @param select Function to select node from set nodes dictionary
         * @param locations Locations to match (empty = no locations)
         * @return New Path builder with node configured
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withNode(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select,
            const std::vector<Location>& locations) &&;

        /**
         * @brief Ignore nodes before specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withAnyNodeBefore(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select) const&;

        /**
         * @brief Ignore nodes before specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withAnyNodeBefore(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select) &&;

        /**
         * @brief Ignore nodes after specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withAnyNodeAfter(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select) const&;

        /**
         * @brief Ignore nodes after specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withAnyNodeAfter(
            std::function<const GmodNode*(
                const std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>>&)> select) &&;

        /**
         * @brief Ignore all location individualizations
         * @return New Path builder with locations ignored
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withoutLocations() const&;

        /**
         * @brief Ignore all location individualizations
         * @return New Path builder with locations ignored
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Path withoutLocations() &&;

        /**
         * @brief Internal match implementation
         * @param other GmodPath to test (nullptr returns false)
         * @return true if path matches the query criteria
         */
        [[nodiscard]] virtual bool match(const GmodPath* other) const override;

    private:
        [[nodiscard]] virtual std::unique_ptr<GmodPathQueryBuilder> clone() const override;
        [[nodiscard]] std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>> resolveSetNodes()
            const;
        [[nodiscard]] std::unordered_map<std::string, const GmodNode*, StringHash, std::equal_to<>> resolveNodes()
            const;

        GmodPath m_gmodPath;
        std::unordered_map<std::string, size_t, StringHash, std::equal_to<>> m_setNodeIndices;
        std::unordered_map<std::string, size_t, StringHash, std::equal_to<>> m_nodeIndices;
    };

    /**
     * @class GmodPathQueryBuilder::Nodes
     * @brief Node-based query builder (build from scratch)
     */
    class GmodPathQueryBuilder::Nodes final : public GmodPathQueryBuilder
    {
        friend class GmodPathQueryBuilder;

    private:
        Nodes() = default;

    public:
        DNV_VISTA_SDK_CPP_API Nodes(const Nodes&);
        DNV_VISTA_SDK_CPP_API Nodes(Nodes&&) noexcept;
        DNV_VISTA_SDK_CPP_API virtual ~Nodes() override;

        DNV_VISTA_SDK_CPP_API Nodes& operator=(const Nodes&);
        DNV_VISTA_SDK_CPP_API Nodes& operator=(Nodes&&) noexcept;

        /**
         * @brief Add node to match with location settings
         * @param node Node to match
         * @param matchAllLocations If true, match any location individualization
         * @return New Nodes builder with node added
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Nodes withNode(const GmodNode& node, bool matchAllLocations = false) const&;

        /**
         * @brief Add node to match with location settings
         * @param node Node to match
         * @param matchAllLocations If true, match any location individualization
         * @return New Nodes builder with node added
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Nodes withNode(const GmodNode& node, bool matchAllLocations = false) &&;

        /**
         * @brief Add node to match with specific locations
         * @param node Node to match
         * @param locations Locations to match (empty = no locations)
         * @return New Nodes builder with node added
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Nodes
        withNode(const GmodNode& node, const std::vector<Location>& locations) const&;

        /**
         * @brief Add node to match with specific locations
         * @param node Node to match
         * @param locations Locations to match (empty = no locations)
         * @return New Nodes builder with node added
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Nodes
        withNode(const GmodNode& node, const std::vector<Location>& locations) &&;

        /**
         * @brief Internal match implementation
         * @param other GmodPath to test (nullptr returns false)
         * @return true if path matches the query criteria
         */
        [[nodiscard]] virtual bool match(const GmodPath* other) const override;

    private:
        [[nodiscard]] virtual std::unique_ptr<GmodPathQueryBuilder> clone() const override;
    };
} // namespace dnv::vista::sdk
