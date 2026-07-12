/**
 * @file GmodPathQuery.h
 * @brief Query API for matching GmodPath instances
 * @details Provides fluent API for building queries to filter GmodPaths based on nodes and locations
 *          Supports path-based queries (starting from existing path) and node-based queries (from scratch)
 */

#pragma once

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

    private:
        /**
         * @brief Construct from a builder
         * @param builder Builder whose state is copied into this query
         */
        explicit GmodPathQuery(const GmodPathQueryBuilder* builder);

    public:
        GmodPathQuery(const GmodPathQuery& other);
        GmodPathQuery(GmodPathQuery&&) noexcept;
        ~GmodPathQuery() = default;

        GmodPathQuery& operator=(const GmodPathQuery& other);
        GmodPathQuery& operator=(GmodPathQuery&&) noexcept;

        /**
         * @brief Match against a GmodPath
         * @param other The GmodPath to test
         * @return true if other matches the query criteria
         */
        [[nodiscard]] bool match(const GmodPath& other) const;

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
        GmodPathQueryBuilder(const GmodPathQueryBuilder&) = default;
        GmodPathQueryBuilder(GmodPathQueryBuilder&&) noexcept = default;
        virtual ~GmodPathQueryBuilder() = default;

        GmodPathQueryBuilder& operator=(const GmodPathQueryBuilder&) = default;
        GmodPathQueryBuilder& operator=(GmodPathQueryBuilder&&) noexcept = default;

        /**
         * @brief Create empty Nodes builder
         * @return New Nodes builder
         */
        [[nodiscard]] static Nodes create();

        /**
         * @brief Create Path builder from existing GmodPath
         * @param path GmodPath to use as base
         * @return New Path builder
         */
        [[nodiscard]] static Path from(const GmodPath& path);

        /**
         * @brief Build immutable query
         * @return GmodPathQuery instance
         */
        [[nodiscard]] GmodPathQuery build() const;

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
            const std::unordered_map<std::string, NodeItem>& filter, const GmodPath& target, bool checkIgnoreFlag);

        std::unordered_map<std::string, NodeItem> m_filter;
    };

    /**
     * @class GmodPathQueryBuilder::Path
     * @brief Path-based query builder
     */
    class GmodPathQueryBuilder::Path final : public GmodPathQueryBuilder
    {
    public:
        Path() = delete;
        explicit Path(const GmodPath& path);
        Path(const Path&) = default;
        Path(Path&&) noexcept = default;
        virtual ~Path() override = default;

        Path& operator=(const Path&) = default;
        Path& operator=(Path&&) noexcept = default;

        /**
         * @brief Get the base GmodPath
         * @return Reference to the GmodPath
         */
        [[nodiscard]] const GmodPath& path() const noexcept;

        /**
         * @brief Configure node to match all locations
         * @param select Function to select node from set nodes dictionary
         * @param matchAllLocations If true, match any location individualization
         * @return New Path builder with node configured
         */
        [[nodiscard]] Path withNode(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select,
            bool matchAllLocations = false) const&;

        /**
         * @brief Configure node to match all locations
         * @param select Function to select node from set nodes dictionary
         * @param matchAllLocations If true, match any location individualization
         * @return New Path builder with node configured
         */
        [[nodiscard]] Path withNode(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select,
            bool matchAllLocations = false) &&;

        /**
         * @brief Configure node with specific locations
         * @param select Function to select node from set nodes dictionary
         * @param locations Locations to match (empty = no locations)
         * @return New Path builder with node configured
         */
        [[nodiscard]] Path withNode(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select,
            const std::vector<Location>& locations) const&;

        /**
         * @brief Configure node with specific locations
         * @param select Function to select node from set nodes dictionary
         * @param locations Locations to match (empty = no locations)
         * @return New Path builder with node configured
         */
        [[nodiscard]] Path withNode(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select,
            const std::vector<Location>& locations) &&;

        /**
         * @brief Ignore nodes before specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] Path withAnyNodeBefore(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select) const&;

        /**
         * @brief Ignore nodes before specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] Path withAnyNodeBefore(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select) &&;

        /**
         * @brief Ignore nodes after specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] Path withAnyNodeAfter(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select) const&;

        /**
         * @brief Ignore nodes after specified node in path
         * @param select Function to select node from full path nodes
         * @return New Path builder with nodes ignored
         * @throws std::invalid_argument if node not in path
         */
        [[nodiscard]] Path withAnyNodeAfter(
            std::function<const GmodNode*(const std::unordered_map<std::string, const GmodNode*>&)> select) &&;

        /**
         * @brief Ignore all location individualizations
         * @return New Path builder with locations ignored
         */
        [[nodiscard]] Path withoutLocations() const&;

        /**
         * @brief Ignore all location individualizations
         * @return New Path builder with locations ignored
         */
        [[nodiscard]] Path withoutLocations() &&;

        /**
         * @brief Internal match implementation
         * @param other GmodPath to test (nullptr returns false)
         * @return true if path matches the query criteria
         */
        [[nodiscard]] virtual bool match(const GmodPath* other) const override;

    private:
        [[nodiscard]] virtual std::unique_ptr<GmodPathQueryBuilder> clone() const override;
        [[nodiscard]] std::unordered_map<std::string, const GmodNode*> resolveSetNodes() const;
        [[nodiscard]] std::unordered_map<std::string, const GmodNode*> resolveNodes() const;

        GmodPath m_gmodPath;
        std::unordered_map<std::string, size_t> m_setNodeIndices;
        std::unordered_map<std::string, size_t> m_nodeIndices;
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
        Nodes(const Nodes&) = default;
        Nodes(Nodes&&) noexcept = default;
        virtual ~Nodes() override = default;

        Nodes& operator=(const Nodes&) = default;
        Nodes& operator=(Nodes&&) noexcept = default;

        /**
         * @brief Add node to match with location settings
         * @param node Node to match
         * @param matchAllLocations If true, match any location individualization
         * @return New Nodes builder with node added
         */
        [[nodiscard]] Nodes withNode(const GmodNode& node, bool matchAllLocations = false) const&;

        /**
         * @brief Add node to match with location settings
         * @param node Node to match
         * @param matchAllLocations If true, match any location individualization
         * @return New Nodes builder with node added
         */
        [[nodiscard]] Nodes withNode(const GmodNode& node, bool matchAllLocations = false) &&;

        /**
         * @brief Add node to match with specific locations
         * @param node Node to match
         * @param locations Locations to match (empty = no locations)
         * @return New Nodes builder with node added
         */
        [[nodiscard]] Nodes withNode(const GmodNode& node, const std::vector<Location>& locations) const&;

        /**
         * @brief Add node to match with specific locations
         * @param node Node to match
         * @param locations Locations to match (empty = no locations)
         * @return New Nodes builder with node added
         */
        [[nodiscard]] Nodes withNode(const GmodNode& node, const std::vector<Location>& locations) &&;

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
