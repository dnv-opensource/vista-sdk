#include "dnv/vista/sdk/transport/ShipId.h"

#include "dnv/vista/sdk/utils/StringUtils.h"

#include <stdexcept>

namespace dnv::vista::sdk::transport
{
    ShipId::ShipId(const ImoNumber& imoNumber)
        : m_value{ imoNumber }
    {}

    ShipId::ShipId(std::string otherId)
        : m_value{ std::move(otherId) }
    {
        if (std::get<std::string>(m_value).empty())
        {
            throw std::invalid_argument{ "ShipId otherId cannot be empty" };
        }
    }

    bool ShipId::operator==(const ShipId& other) const noexcept
    {
        return m_value == other.m_value;
    }

    bool ShipId::isImoNumber() const noexcept
    {
        return std::holds_alternative<ImoNumber>(m_value);
    }

    bool ShipId::isOtherId() const noexcept
    {
        return std::holds_alternative<std::string>(m_value);
    }

    std::optional<ImoNumber> ShipId::imoNumber() const noexcept
    {
        if (std::holds_alternative<ImoNumber>(m_value))
        {
            return std::get<ImoNumber>(m_value);
        }
        return std::nullopt;
    }

    std::optional<std::string_view> ShipId::otherId() const noexcept
    {
        if (std::holds_alternative<std::string>(m_value))
        {
            return std::get<std::string>(m_value);
        }
        return std::nullopt;
    }

    std::string ShipId::toString() const
    {
        return std::visit(
            [](const auto& value) -> std::string {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, ImoNumber>)
                {
                    return value.toString();
                }
                else
                {
                    return value;
                }
            },
            m_value);
    }

    std::optional<ShipId> ShipId::fromString(std::string_view value) noexcept
    {
        try
        {
            if (string::isNullOrWhiteSpace(value))
            {
                return std::nullopt;
            }

            if (value.length() >= 3 && string::iequals(value.substr(0, 3), "IMO"))
            {
                auto imoOpt = ImoNumber::fromString(value);
                if (imoOpt.has_value())
                {
                    return std::make_optional(ShipId{ *imoOpt });
                }
            }

            return std::make_optional(ShipId{ std::string{ value } });
        }
        catch (const std::bad_alloc&)
        {
            return std::nullopt;
        }
    }
} // namespace dnv::vista::sdk::transport
