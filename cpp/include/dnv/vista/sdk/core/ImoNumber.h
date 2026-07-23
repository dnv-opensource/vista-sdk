/**
 * @file ImoNumber.h
 * @brief IMO number representation and validation
 * @details Represents an IMO (International Maritime Organization) number with validation
 *          IMO numbers are unique identifiers for ships, consisting of the prefix "IMO"
 *          followed by a seven-digit number with checksum validation
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include <optional>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    /**
     * @brief Represents an International Maritime Organization (IMO) number
     *
     * An IMO number is a unique seven-digit identifier assigned to maritime vessels,
     * featuring a check digit for validation according to the IMO standard
     * This class provides functionality to create, validate, parse, and represent IMO numbers
     * Instances of this class are immutable once constructed
     */
    class ImoNumber final
    {
    private:
        struct ValidTag
        {
        };
        constexpr ImoNumber(int value, ValidTag) noexcept
            : m_value{ value }
        {}

    public:
        ImoNumber() = delete;

        /**
         * @brief Construct IMO number from integer with validation
         * @param value Seven-digit IMO number (e.g., 9074729)
         * @throws std::invalid_argument if value fails checksum validation
         */
        DNV_VISTA_SDK_CPP_API explicit ImoNumber(int value);

        /**
         * @brief Construct IMO number from string with validation
         * @param value IMO number string with or without "IMO" prefix (e.g., "IMO9074729" or "9074729")
         * @throws std::invalid_argument if string format is invalid or checksum fails
         */
        DNV_VISTA_SDK_CPP_API explicit ImoNumber(std::string_view value);

        ImoNumber(const ImoNumber&) = default;
        ImoNumber(ImoNumber&&) noexcept = default;
        ~ImoNumber() = default;

        ImoNumber& operator=(const ImoNumber&) = default;
        ImoNumber& operator=(ImoNumber&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const ImoNumber& other) const noexcept;

        /**
         * @brief Checks if an integer value represents a valid IMO number
         *
         * This method validates the 7-digit structure and the checksum
         * @param imoNumber The integer value to check (e.g., 9074729)
         * @return True if the integer is a valid IMO number, false otherwise
         */
        [[nodiscard]] static constexpr bool isValid(int imoNumber) noexcept;

        /**
         * @brief Get the raw integer value
         * @return Seven-digit integer (e.g., 9074729)
         */
        [[nodiscard]] inline constexpr int value() const noexcept;

        /**
         * @brief Parse IMO number from string and return optional
         * @param value String to parse (e.g., "IMO9074729" or "9074729")
         * @return Optional ImoNumber if valid, nullopt otherwise
         * @details Accepts strings with or without "IMO" prefix and validates checksum
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API std::optional<ImoNumber> fromString(std::string_view value) noexcept;

        /**
         * @brief Convert to string with "IMO" prefix
         * @return String in format "IMO" followed by 7 digits (e.g., "IMO9074729")
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API std::string toString() const;

    private:
        int m_value;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/ImoNumber.inl"
