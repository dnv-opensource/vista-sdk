/**
 * @file LocationBuilder.h
 * @brief Builder pattern for constructing VIS Location instances
 * @details Provides a fluent API for building Location objects with validation
 *          Supports setting individual location components (number, side, vertical, transverse, longitudinal)
 *          and validates values against the VIS version's location specification
 */

#pragma once

#include "Locations.h"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace dnv::vista::sdk
{
    enum class VisVersion : std::uint8_t;

    /**
     * @class LocationBuilder
     * @brief Builder for constructing VIS Location instances with fluent API
     * @details Provides immutable builder pattern for creating Location objects with validation
     *          Each with* method returns a new builder instance with the modified value,
     *          preserving immutability. Values are validated against the VIS version specification
     *          Use build() to construct the final Location object from the builder state
     */
    class LocationBuilder final
    {
        friend class Locations;

    private:
        /**
         * @brief Constructs a LocationBuilder for a given VIS version
         * @param visVersion VIS version for validation
         * @param reversedGroups Non-owning pointer to the reversed groups map from Locations
         */
        inline explicit LocationBuilder(
            VisVersion visVersion, const std::unordered_map<char, LocationGroup>* reversedGroups);

    public:
        LocationBuilder() = delete;
        LocationBuilder(const LocationBuilder&) = default;
        LocationBuilder(LocationBuilder&&) noexcept = default;
        ~LocationBuilder() = default;

        LocationBuilder& operator=(const LocationBuilder&) = default;
        LocationBuilder& operator=(LocationBuilder&&) noexcept = default;

        /**
         * @brief Create a new LocationBuilder for the given Locations instance
         * @param locations Locations instance to build for
         * @return New LocationBuilder instance
         */
        [[nodiscard]] static inline LocationBuilder create(const Locations& locations);

        /** @brief Get the VIS version for this builder */
        [[nodiscard]] inline VisVersion version() const noexcept;

        /** @brief Get the location number component (nullopt if not set) */
        [[nodiscard]] inline std::optional<int> number() const noexcept;

        /** @brief Get the side location component (nullopt if not set) */
        [[nodiscard]] inline std::optional<char> side() const noexcept;

        /** @brief Get the vertical location component (nullopt if not set) */
        [[nodiscard]] inline std::optional<char> vertical() const noexcept;

        /** @brief Get the transverse location component (nullopt if not set) */
        [[nodiscard]] inline std::optional<char> transverse() const noexcept;

        /** @brief Get the longitudinal location component (nullopt if not set) */
        [[nodiscard]] inline std::optional<char> longitudinal() const noexcept;

        /**
         * @brief Parse and set location components from existing Location
         * @param location Location to parse and extract components from
         * @return New LocationBuilder with components from the parsed location
         */
        [[nodiscard]] LocationBuilder withLocation(const Location& location) const&;

        /**
         * @brief Parse and set location components from existing Location
         * @param location Location to parse and extract components from
         * @return New LocationBuilder with components from the parsed location
         */
        [[nodiscard]] LocationBuilder withLocation(const Location& location) &&;

        /**
         * @brief Parse and set location components from optional Location
         * @param location Optional location to parse and extract components from
         * @return New LocationBuilder with components from the parsed location
         * @throws std::invalid_argument if location is std::nullopt
         */
        [[nodiscard]] inline LocationBuilder withLocation(const std::optional<Location>& location) const&;

        /**
         * @brief Parse and set location components from optional Location
         * @param location Optional location to parse and extract components from
         * @return New LocationBuilder with components from the parsed location
         * @throws std::invalid_argument if location is std::nullopt
         */
        [[nodiscard]] inline LocationBuilder withLocation(const std::optional<Location>& location) &&;

        /**
         * @brief Set the number component
         * @param number Location number (must be > 0)
         * @return New LocationBuilder with the number component set
         * @throws std::invalid_argument if number is less than 1
         */
        [[nodiscard]] inline LocationBuilder withNumber(int number) const&;

        /**
         * @brief Set the number component
         * @param number Location number (must be > 0)
         * @return New LocationBuilder with the number component set
         * @throws std::invalid_argument if number is less than 1
         */
        [[nodiscard]] inline LocationBuilder withNumber(int number) &&;

        /**
         * @brief Remove the number component
         * @return New LocationBuilder with the number component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutNumber() const&;

        /**
         * @brief Remove the number component
         * @return New LocationBuilder with the number component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutNumber() &&;

        /**
         * @brief Set location component by character code
         * @param code Character code (e.g., 'S' for Side, 'U' for Upper)
         * @return New LocationBuilder with the appropriate component set
         * @throws std::invalid_argument if character is not valid for any location group
         * @details Automatically determines which location group the character belongs to
         *          and calls the appropriate with* method (withSide, withVertical, etc.)
         */
        [[nodiscard]] inline LocationBuilder withCode(char code) const&;

        /**
         * @brief Set location component by character code
         * @param code Character code (e.g., 'S' for Side, 'U' for Upper)
         * @return New LocationBuilder with the appropriate component set
         * @throws std::invalid_argument if character is not valid for any location group
         * @details Automatically determines which location group the character belongs to
         *          and calls the appropriate with* method (withSide, withVertical, etc.)
         */
        [[nodiscard]] inline LocationBuilder withCode(char code) &&;

        /**
         * @brief Set the side component
         * @param side Side character (must be valid for VIS version)
         * @return New LocationBuilder with the side component set
         * @throws std::invalid_argument if character is invalid for side group
         */
        [[nodiscard]] inline LocationBuilder withSide(char side) const&;

        /**
         * @brief Set the side component
         * @param side Side character (must be valid for VIS version)
         * @return New LocationBuilder with the side component set
         * @throws std::invalid_argument if character is invalid for side group
         */
        [[nodiscard]] inline LocationBuilder withSide(char side) &&;

        /**
         * @brief Remove the side component
         * @return New LocationBuilder with the side component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutSide() const&;

        /**
         * @brief Remove the side component
         * @return New LocationBuilder with the side component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutSide() &&;

        /**
         * @brief Set the vertical component
         * @param vertical Vertical character (must be valid for VIS version)
         * @return New LocationBuilder with the vertical component set
         * @throws std::invalid_argument if character is invalid for vertical group
         */
        [[nodiscard]] inline LocationBuilder withVertical(char vertical) const&;

        /**
         * @brief Set the vertical component
         * @param vertical Vertical character (must be valid for VIS version)
         * @return New LocationBuilder with the vertical component set
         * @throws std::invalid_argument if character is invalid for vertical group
         */
        [[nodiscard]] inline LocationBuilder withVertical(char vertical) &&;

        /**
         * @brief Remove the vertical component
         * @return New LocationBuilder with the vertical component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutVertical() const&;

        /**
         * @brief Remove the vertical component
         * @return New LocationBuilder with the vertical component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutVertical() &&;

        /**
         * @brief Set the transverse component
         * @param transverse Transverse character (must be valid for VIS version)
         * @return New LocationBuilder with the transverse component set
         * @throws std::invalid_argument if character is invalid for transverse group
         */
        [[nodiscard]] inline LocationBuilder withTransverse(char transverse) const&;

        /**
         * @brief Set the transverse component
         * @param transverse Transverse character (must be valid for VIS version)
         * @return New LocationBuilder with the transverse component set
         * @throws std::invalid_argument if character is invalid for transverse group
         */
        [[nodiscard]] inline LocationBuilder withTransverse(char transverse) &&;

        /**
         * @brief Remove the transverse component
         * @return New LocationBuilder with the transverse component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutTransverse() const&;

        /**
         * @brief Remove the transverse component
         * @return New LocationBuilder with the transverse component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutTransverse() &&;

        /**
         * @brief Set the longitudinal component
         * @param longitudinal Longitudinal character (must be valid for VIS version)
         * @return New LocationBuilder with the longitudinal component set
         * @throws std::invalid_argument if character is invalid for longitudinal group
         */
        [[nodiscard]] inline LocationBuilder withLongitudinal(char longitudinal) const&;

        /**
         * @brief Set the longitudinal component
         * @param longitudinal Longitudinal character (must be valid for VIS version)
         * @return New LocationBuilder with the longitudinal component set
         * @throws std::invalid_argument if character is invalid for longitudinal group
         */
        [[nodiscard]] inline LocationBuilder withLongitudinal(char longitudinal) &&;

        /**
         * @brief Remove the longitudinal component
         * @return New LocationBuilder with the longitudinal component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutLongitudinal() const&;

        /**
         * @brief Remove the longitudinal component
         * @return New LocationBuilder with the longitudinal component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutLongitudinal() &&;

        /**
         * @brief Remove location component by group
         * @param group The location group component to remove
         * @return New LocationBuilder with the specified component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutValue(LocationGroup group) const&;

        /**
         * @brief Remove location component by group
         * @param group The location group component to remove
         * @return New LocationBuilder with the specified component cleared
         */
        [[nodiscard]] inline LocationBuilder withoutValue(LocationGroup group) &&;

        /**
         * @brief Build the final Location object from builder state
         * @return Location instance constructed from current builder state
         */
        [[nodiscard]] inline Location build() const;

        /**
         * @brief Convert builder state to string representation
         * @return String representation of the location with components in sorted order
         */
        [[nodiscard]] inline std::string toString() const;

    private:
        std::optional<int> m_number;                                     ///< Optional number component
        std::optional<char> m_side;                                      ///< Optional side component
        std::optional<char> m_vertical;                                  ///< Optional vertical component
        std::optional<char> m_transverse;                                ///< Optional transverse component
        std::optional<char> m_longitudinal;                              ///< Optional longitudinal component
        VisVersion m_visVersion;                                         ///< VIS version for validation
        const std::unordered_map<char, LocationGroup>* m_reversedGroups; ///< Non-owning pointer to reversed groups map
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/LocationBuilder.inl"
