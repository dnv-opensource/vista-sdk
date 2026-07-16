/**
 * @file DataChannelId.h
 * @brief Data channel identifier for ISO 19848 time series data
 */

#pragma once

#include "dnv/vista/sdk/core/LocalId.h"

#include <functional>
#include <optional>
#include <string>
#include <variant>

namespace dnv::vista::sdk::transport::timeseries
{
    /**
     * @brief Polymorphic data channel identifier supporting LocalId or ShortId
     * @details Type-safe sum type that can contain either:
     *          - LocalId: full VIS local identifier
     *          - ShortId: short string identifier
     */
    class DataChannelId final
    {
    private:
        /**
         * @brief Constructs DataChannelId from LocalId
         * @param localId Local identifier
         */
        explicit DataChannelId(LocalId localId) noexcept;

        /**
         * @brief Constructs DataChannelId from string_view identifier
         * @param shortId Short identifier string
         */
        explicit DataChannelId(std::string shortId) noexcept;

    public:
        DataChannelId() = delete;
        DataChannelId(const DataChannelId&) = default;
        DataChannelId(DataChannelId&&) noexcept = default;
        ~DataChannelId() = default;

        DataChannelId& operator=(const DataChannelId&) = default;
        DataChannelId& operator=(DataChannelId&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const DataChannelId& other) const noexcept;

        /**
         * @brief Check if this contains a LocalId
         * @return true if contains LocalId, false if contains ShortId
         */
        [[nodiscard]] inline bool isLocalId() const noexcept;

        /**
         * @brief Check if this contains a ShortId
         * @return true if contains ShortId, false if contains LocalId
         */
        [[nodiscard]] inline bool isShortId() const noexcept;

        /**
         * @brief Get LocalId if present
         * @return Optional LocalId reference
         */
        [[nodiscard]] inline std::optional<std::reference_wrapper<const LocalId>> localId() const noexcept;

        /**
         * @brief Get ShortId if present
         * @return Optional ShortId string view
         */
        [[nodiscard]] inline std::optional<std::string_view> shortId() const noexcept;

        /**
         * @brief Convert to string representation
         * @return String representation (LocalId.toString() or ShortId)
         */
        [[nodiscard]] inline std::string toString() const;

        /**
         * @brief Create DataChannelId from string representation
         * @param value String to parse
         * @return Optional containing DataChannelId if parsing succeeded, nullopt otherwise
         * @details Attempts to parse value as a LocalId. If that fails (e.g. malformed VIS
         *          syntax, or a string that isn't intended as a LocalId at all), value is stored
         *          verbatim as a ShortId instead. There is no '/'-prefix check - a string
         *          starting with '/' that fails to parse as a valid LocalId still becomes a
         *          ShortId
         * @note Only an empty value returns nullopt. Any other failure to parse as LocalId
         *       silently falls back to ShortId rather than failing outright
         */
        [[nodiscard]] static std::optional<DataChannelId> fromString(std::string_view value) noexcept;

        /**
         * @brief Pattern matching visitor for DataChannelId
         * @tparam F1 Function type for LocalId case
         * @tparam F2 Function type for ShortId case
         * @param onLocalId Function to call if DataChannelId contains LocalId
         * @param onShortId Function to call if DataChannelId contains ShortId
         * @return Result of the matching function
         */
        template <typename F1, typename F2>
        [[nodiscard]] inline auto match(F1&& onLocalId, F2&& onShortId) const;

    private:
        std::variant<LocalId, std::string> m_value; ///< Either the LocalId or the ShortId, never both
    };
} // namespace dnv::vista::sdk::transport::timeseries

#include "dnv/vista/sdk/detail/transport/timeseries/DataChannelId.inl"
