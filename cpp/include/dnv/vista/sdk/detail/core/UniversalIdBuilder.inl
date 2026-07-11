namespace dnv::vista::sdk
{
    inline bool UniversalIdBuilder::operator==(const UniversalIdBuilder& other) const noexcept
    {
        return m_imoNumber == other.m_imoNumber && m_localIdBuilder == other.m_localIdBuilder;
    }

    inline constexpr std::string_view UniversalIdBuilder::namingEntity() noexcept
    {
        return internal::iso19848::annexC::NamingEntity;
    }

    inline const std::optional<ImoNumber>& UniversalIdBuilder::imoNumber() const noexcept
    {
        return m_imoNumber;
    }

    inline const std::optional<LocalIdBuilder>& UniversalIdBuilder::localId() const noexcept
    {
        return m_localIdBuilder;
    }

    inline bool UniversalIdBuilder::isValid() const noexcept
    {
        return m_imoNumber.has_value() && m_localIdBuilder.has_value() && m_localIdBuilder->isValid();
    }

    inline UniversalIdBuilder UniversalIdBuilder::create(VisVersion visVersion) noexcept
    {
        UniversalIdBuilder builder;
        builder.m_localIdBuilder = LocalIdBuilder::create(visVersion);
        return builder;
    }

    inline UniversalIdBuilder UniversalIdBuilder::withImoNumber(const ImoNumber& imoNumber) const&
    {
        auto copy = *this;
        copy.m_imoNumber = imoNumber;
        return copy;
    }

    inline UniversalIdBuilder UniversalIdBuilder::withImoNumber(const ImoNumber& imoNumber) &&
    {
        m_imoNumber = imoNumber;
        return std::move(*this);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withImoNumber(const std::optional<ImoNumber>& imoNumber) const&
    {
        if (!imoNumber.has_value())
        {
            throw std::invalid_argument{ "ImoNumber cannot be nullopt" };
        }
        return withImoNumber(*imoNumber);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withImoNumber(const std::optional<ImoNumber>& imoNumber) &&
    {
        if (!imoNumber.has_value())
        {
            throw std::invalid_argument{ "ImoNumber cannot be nullopt" };
        }
        return std::move(*this).withImoNumber(*imoNumber);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withoutImoNumber() const&
    {
        auto copy = *this;
        copy.m_imoNumber = std::nullopt;
        return copy;
    }

    inline UniversalIdBuilder UniversalIdBuilder::withoutImoNumber() && noexcept
    {
        m_imoNumber = std::nullopt;
        return std::move(*this);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withLocalId(const LocalIdBuilder& localIdBuilder) const&
    {
        auto copy = *this;
        copy.m_localIdBuilder = localIdBuilder;
        return copy;
    }

    inline UniversalIdBuilder UniversalIdBuilder::withLocalId(const LocalIdBuilder& localIdBuilder) &&
    {
        m_localIdBuilder = localIdBuilder;
        return std::move(*this);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withLocalId(
        const std::optional<LocalIdBuilder>& localIdBuilder) const&
    {
        if (!localIdBuilder.has_value())
        {
            throw std::invalid_argument{ "LocalIdBuilder cannot be nullopt" };
        }
        return withLocalId(*localIdBuilder);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withLocalId(const std::optional<LocalIdBuilder>& localIdBuilder) &&
    {
        if (!localIdBuilder.has_value())
        {
            throw std::invalid_argument{ "LocalIdBuilder cannot be nullopt" };
        }
        return std::move(*this).withLocalId(*localIdBuilder);
    }

    inline UniversalIdBuilder UniversalIdBuilder::withoutLocalId() const&
    {
        auto copy = *this;
        copy.m_localIdBuilder = std::nullopt;
        return copy;
    }

    inline UniversalIdBuilder UniversalIdBuilder::withoutLocalId() && noexcept
    {
        m_localIdBuilder = std::nullopt;
        return std::move(*this);
    }

    inline void UniversalIdBuilder::toString(std::string& out) const
    {
        if (!m_imoNumber.has_value())
        {
            throw std::invalid_argument{ "Invalid Universal Id state: Missing IMO Number" };
        }
        if (!m_localIdBuilder.has_value())
        {
            throw std::invalid_argument{ "Invalid Universal Id state: Missing LocalId" };
        }

        out += internal::iso19848::annexC::NamingEntity;
        out += '/';
        out += m_imoNumber->toString();

        m_localIdBuilder->toString(out);
    }

    inline std::string UniversalIdBuilder::toString() const
    {
        std::string out;
        toString(out);
        return out;
    }
} // namespace dnv::vista::sdk
