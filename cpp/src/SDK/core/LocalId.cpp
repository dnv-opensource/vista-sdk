#include "dnv/vista/sdk/core/LocalId.h"

#include "dnv/vista/sdk/core/LocalIdBuilder.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    LocalId::LocalId(const LocalIdBuilder& builder)
        : m_builder{ builder }
    {
        if (m_builder.isEmpty())
        {
            throw std::invalid_argument{ "LocalId cannot be constructed from empty LocalIdBuilder" };
        }
        if (!m_builder.isValid())
        {
            throw std::invalid_argument{ "LocalId cannot be constructed from invalid LocalIdBuilder" };
        }
    }

    std::optional<LocalId> LocalId::fromString(std::string_view localIdStr) noexcept
    {
        return LocalIdBuilder::fromString(localIdStr);
    }

    std::optional<LocalId> LocalId::fromString(std::string_view localIdStr, ParsingErrors& errors) noexcept
    {
        return LocalIdBuilder::fromString(localIdStr, errors);
    }

    namespace mqtt
    {
        LocalId::LocalId(const sdk::LocalIdBuilder& builder)
            : m_builder{ builder }
        {
            if (m_builder.isEmpty())
            {
                throw std::invalid_argument{ "LocalId cannot be constructed from empty LocalIdBuilder" };
            }
            if (!m_builder.isValid())
            {
                throw std::invalid_argument{ "LocalId cannot be constructed from invalid LocalIdBuilder" };
            }
        }
    } // namespace mqtt
} // namespace dnv::vista::sdk
