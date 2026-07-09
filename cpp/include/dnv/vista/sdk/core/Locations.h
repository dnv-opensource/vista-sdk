/**
 * @file Locations.h
 * @brief VIS location representation with validation and parsing
 * @details Provides Location, RelativeLocation, and Locations classes for parsing and validating
 *          VIS location strings. Locations are used to identify physical positions on vessels
 *          with numeric prefixes and alphabetically sorted location codes
 */

#pragma once

#include "VisVersions.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dnv::vista::sdk
{
    struct LocationsDto;
    class ParsingErrors;

    namespace internal
    {
        class LocationParsingErrorBuilder;
    } // namespace internal

    /**
     * @brief Classification of VIS location codes into functional groups
     * @details Groups location codes by their semantic meaning (Number, Side, Vertical,
     *          Transverse, Longitudinal). Used for organizing and validating location strings
     */
    enum class LocationGroup : std::uint8_t
    {
        Number = 0,  ///< Numeric location prefix group
        Side,        ///< Port/Center/Starboard group
        Vertical,    ///< Upper/Middle/Lower group
        Transverse,  ///< Inboard/Outboard group
        Longitudinal ///< Forward/Aft group
    };

    /**
     * @brief Represents a VIS location value
     * @details Lightweight value type that owns the canonical string representation of a VIS
     *          location (for example "1PS"). Instances are constructed by `Locations` after
     *          validation and are intended to be used as an immutable value object
     */
    class Location final
    {
        friend class Locations;

    private:
        /**
         * @brief Constructs a Location from a validated string
         * @param value Validated location string
         */
        inline explicit Location(std::string_view value) noexcept;

    public:
        Location() = delete;
        Location(const Location&) noexcept = default;
        Location(Location&&) noexcept = default;
        ~Location() = default;

        Location& operator=(const Location& other) noexcept = default;
        Location& operator=(Location&& other) noexcept = default;

        /**
         * @brief Equality comparison operator
         * @param other Location to compare with
         * @return true if location values are equal
         */
        [[nodiscard]] inline bool operator==(const Location& other) const noexcept;

        /** @brief Get the location string value */
        [[nodiscard]] inline const std::string& value() const noexcept;

    private:
        std::string m_value; ///< Canonical location string representation
    };

    /**
     * @brief Metadata for a relative location code
     * @details Associates a single-character location code with a human readable name,
     *          its canonical `Location` representation and an optional definition
     */
    class RelativeLocation final
    {
        friend class Locations;

    private:
        /**
         * @brief Constructs a RelativeLocation
         * @param code Single character location code
         * @param name Human-readable name
         * @param location Canonical Location representation
         * @param definition Optional definition text
         */
        inline explicit RelativeLocation(
            char code,
            std::string_view name,
            const Location& location,
            std::optional<std::string> definition = std::nullopt) noexcept;

    public:
        RelativeLocation() = delete;
        RelativeLocation(const RelativeLocation&) = default;
        RelativeLocation(RelativeLocation&&) noexcept = default;
        ~RelativeLocation() = default;

        RelativeLocation& operator=(const RelativeLocation& other) = default;
        RelativeLocation& operator=(RelativeLocation&& other) noexcept = default;

        /**
         * @brief Equality comparison based on code
         * @param other RelativeLocation to compare with
         * @return true if codes are equal
         */
        [[nodiscard]] inline bool operator==(const RelativeLocation& other) const noexcept;

        /** @brief Get the single-character location code */
        [[nodiscard]] inline char code() const noexcept;

        /** @brief Get the human-readable name */
        [[nodiscard]] inline const std::string& name() const noexcept;

        /** @brief Get the optional definition (nullopt if not set) */
        [[nodiscard]] inline const std::optional<std::string>& definition() const noexcept;

        /** @brief Get the canonical Location representation */
        [[nodiscard]] inline const Location& location() const noexcept;

    private:
        char m_code;                             ///< Single-character location code
        std::string m_name;                      ///< Human-readable name
        Location m_location;                     ///< Canonical Location representation
        std::optional<std::string> m_definition; ///< Optional definition text
    };

    /**
     * @brief Collection and parser for VIS locations
     * @details Constructs a set of known `RelativeLocation` items for a given VIS version,
     *          provides grouping and parsing utilities (see `fromString`)
     */
    class Locations final
    {
        friend class VIS;

    private:
        /**
         * @brief Constructs Locations from DTO
         * @param version VIS version
         * @param dto LocationsDto containing location definitions
         */
        explicit Locations(VisVersion version, const LocationsDto& dto);

    public:
        Locations() = delete;
        Locations(const Locations&) = default;
        Locations(Locations&&) noexcept = default;
        ~Locations() = default;

        Locations& operator=(const Locations&) = default;
        Locations& operator=(Locations&&) noexcept = default;

        /** @brief Get the VIS version */
        inline VisVersion version() const noexcept;

        /** @brief Get all relative locations */
        inline const std::vector<RelativeLocation>& relativeLocations() const noexcept;

        /** @brief Get relative locations grouped by LocationGroup */
        inline const std::unordered_map<LocationGroup, std::vector<RelativeLocation>>& groups() const noexcept;

        /**
         * @brief Parse location string without error details
         * @param locationStr Location string to parse
         * @return Optional Location if valid, nullopt otherwise
         */
        [[nodiscard]] std::optional<Location> fromString(std::string_view locationStr) const noexcept;

        /**
         * @brief Parse location string with error details
         * @param locationStr Location string to parse
         * @param errors Output ParsingErrors containing detailed error information
         * @return Optional Location if valid, nullopt otherwise
         */
        [[nodiscard]] std::optional<Location> fromString(
            std::string_view locationStr, ParsingErrors& errors) const noexcept;

    private:
        std::optional<Location> fromString(
            std::string_view locationStr, internal::LocationParsingErrorBuilder* errorBuilder) const noexcept;

        std::unordered_map<LocationGroup, std::vector<RelativeLocation>> m_groups; ///< Location groups map
        std::unordered_map<char, LocationGroup> m_reversedGroups;                  ///< Code to group mapping
        std::unordered_set<char> m_locationCodes;                                  ///< All valid location codes
        std::vector<RelativeLocation> m_relativeLocations;                         ///< All relative locations
        VisVersion m_visVersion;                                                   ///< VIS version
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/Locations.inl"
