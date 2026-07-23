/**
 * @file ShipId.h
 * @brief Ship identifier representation for ISO 19848
 * @details Represents a ship identifier that can be either an IMO number or an alternative identifier
 *          This is used in the Header of DataChannelList and TimeSeriesData packages
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "dnv/vista/sdk/core/ImoNumber.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace dnv::vista::sdk::transport
{
    /**
     * @brief Represents a ship identifier in ISO 19848 format
     *
     * A ShipId is a discriminated union that can hold either:
     * - An IMO number (preferred, validated identifier)
     * - An alternative string identifier (for vessels without IMO numbers)
     *
     * In ISO 19848, IMO numbers as ShipID should be prefixed with "IMO"
     */
    class ShipId final
    {
    public:
        /**
         * @brief Construct ShipId from IMO number
         * @param imoNumber Valid IMO number
         */
        DNV_VISTA_SDK_CPP_API explicit ShipId(const ImoNumber& imoNumber);

        /**
         * @brief Construct ShipId from alternative identifier
         * @param otherId Alternative ship identifier string
         * @throws std::invalid_argument if otherId is empty
         */
        DNV_VISTA_SDK_CPP_API explicit ShipId(std::string otherId);

        ShipId() = delete;
        ShipId(const ShipId&) = default;
        ShipId(ShipId&&) noexcept = default;
        ~ShipId() = default;

        ShipId& operator=(const ShipId&) = default;
        ShipId& operator=(ShipId&&) noexcept = default;

        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool operator==(const ShipId& other) const noexcept;

        /**
         * @brief Check if ShipId contains an IMO number
         * @return true if this ShipId holds an IMO number
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isImoNumber() const noexcept;

        /**
         * @brief Check if ShipId contains an alternative identifier
         * @return true if this ShipId holds an alternative identifier
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isOtherId() const noexcept;

        /**
         * @brief Get IMO number if available
         * @return Optional containing IMO number, or nullopt if ShipId holds alternative ID
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API std::optional<ImoNumber> imoNumber() const noexcept;

        /**
         * @brief Get alternative identifier if available
         * @return Optional containing alternative ID, or nullopt if ShipId holds IMO number
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API std::optional<std::string_view> otherId() const noexcept;

        /**
         * @brief Pattern matching visitor for ShipId
         * @tparam F1 Function type for IMO number case
         * @tparam F2 Function type for other ID case
         * @param onImoNumber Function to call if ShipId contains IMO number
         * @param onOtherId Function to call if ShipId contains alternative ID
         * @return Result of the matching function
         */
        template <typename F1, typename F2>
        [[nodiscard]] auto match(F1&& onImoNumber, F2&& onOtherId) const;

        /**
         * @brief Convert ShipId to string representation
         * @return String representation (IMO number with "IMO" prefix, or alternative ID)
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API std::string toString() const;

        /**
         * @brief Create ShipId from string representation
         * @param value String containing "IMO" prefix followed by IMO number, or alternative ID
         * @return Optional containing ShipId if parsing succeeded, nullopt otherwise
         * @details If the string starts with "IMO" (case-insensitive) and is followed by a valid
         *          IMO number, creates a ShipId with ImoNumber. Otherwise, creates a ShipId with
         *          the alternative identifier. Returns nullopt if value is empty
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API std::optional<ShipId> fromString(std::string_view value) noexcept;

    private:
        std::variant<ImoNumber, std::string> m_value;
    };

    template <typename F1, typename F2>
    inline auto ShipId::match(F1&& onImoNumber, F2&& onOtherId) const
    {
        return std::visit(
            [&](const auto& value) -> decltype(auto) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, ImoNumber>)
                {
                    return std::forward<F1>(onImoNumber)(value);
                }
                else
                {
                    return std::forward<F2>(onOtherId)(value);
                }
            },
            m_value);
    }
} // namespace dnv::vista::sdk::transport
