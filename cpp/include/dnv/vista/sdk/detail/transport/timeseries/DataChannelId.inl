namespace dnv::vista::sdk::transport::timeseries
{
    inline DataChannelId::DataChannelId(LocalId localId) noexcept
        : m_value{ std::move(localId) }
    {}

    inline DataChannelId::DataChannelId(std::string shortId) noexcept
        : m_value{ std::move(shortId) }
    {}

    inline bool DataChannelId::operator==(const DataChannelId& other) const noexcept
    {
        return m_value == other.m_value;
    }

    inline bool DataChannelId::isLocalId() const noexcept
    {
        return std::holds_alternative<LocalId>(m_value);
    }

    inline bool DataChannelId::isShortId() const noexcept
    {
        return std::holds_alternative<std::string>(m_value);
    }

    inline std::optional<std::reference_wrapper<const LocalId>> DataChannelId::localId() const noexcept
    {
        if (const auto* ptr = std::get_if<LocalId>(&m_value))
        {
            return std::cref(*ptr);
        }

        return std::nullopt;
    }

    inline std::optional<std::string_view> DataChannelId::shortId() const noexcept
    {
        if (const auto* ptr = std::get_if<std::string>(&m_value))
        {
            return *ptr;
        }

        return std::nullopt;
    }

    inline std::string DataChannelId::toString() const
    {
        return std::visit(
            [](const auto& value) -> std::string {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, LocalId>)
                    return value.toString();
                else
                    return std::string{ value };
            },
            m_value);
    }

    template <typename F1, typename F2>
    inline auto DataChannelId::match(F1&& onLocalId, F2&& onShortId) const
    {
        return std::visit(
            [&](const auto& value) -> decltype(auto) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, LocalId>)
                {
                    return std::forward<F1>(onLocalId)(value);
                }
                else
                {
                    return std::forward<F2>(onShortId)(value);
                }
            },
            m_value);
    }
} // namespace dnv::vista::sdk::transport::timeseries
