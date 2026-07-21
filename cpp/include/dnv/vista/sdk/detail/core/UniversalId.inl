#include "ISO19848AnnexC.h"

#include <cassert>

namespace dnv::vista::sdk
{
    inline bool UniversalId::operator==(const UniversalId& other) const noexcept
    {
        return m_builder == other.m_builder;
    }

    inline constexpr std::string_view UniversalId::namingEntity() noexcept
    {
        return internal::iso19848::annexC::NamingEntity;
    }

    inline const ImoNumber& UniversalId::imoNumber() const noexcept
    {
        assert(m_builder.imoNumber().has_value());
        return *m_builder.imoNumber();
    }

    inline const LocalId& UniversalId::localId() const noexcept
    {
        return m_localId;
    }

    inline const UniversalIdBuilder& UniversalId::builder() const noexcept
    {
        return m_builder;
    }

    inline std::string UniversalId::toString() const&
    {
        return m_builder.toString();
    }

    inline std::string UniversalId::toString() &&
    {
        return std::move(m_builder).toString();
    }

    inline void UniversalId::toString(StringBuilder& builder) const
    {
        m_builder.toString(builder);
    }
} // namespace dnv::vista::sdk
