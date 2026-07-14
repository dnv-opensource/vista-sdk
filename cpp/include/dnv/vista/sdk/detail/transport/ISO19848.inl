namespace dnv::vista::sdk::transport
{
    inline DataChannelTypeName::DataChannelTypeName(std::string type, std::string description)
        : type{ std::move(type) },
          description{ std::move(description) }
    {}

    inline DataChannelTypeNames::DataChannelTypeNames(std::vector<DataChannelTypeName> values)
        : m_values(std::move(values))
    {}

    inline DataChannelTypeNames::ConstIterator DataChannelTypeNames::begin() const noexcept
    {
        return m_values.begin();
    }

    inline DataChannelTypeNames::ConstIterator DataChannelTypeNames::end() const noexcept
    {
        return m_values.end();
    }

    inline size_t DataChannelTypeNames::size() const noexcept
    {
        return m_values.size();
    }

    inline bool DataChannelTypeNames::isEmpty() const noexcept
    {
        return m_values.empty();
    }

    inline Value::Decimal::Decimal(std::string_view str)
        : value{ str }
    {}

    inline Value::Decimal::Decimal(float f)
        : value{ std::move(f) }
    {}

    inline Value::Decimal::Decimal(double d)
        : value{ std::move(d) }
    {}

    inline Value::Decimal::Decimal(sdk::Decimal d)
        : value{ std::move(d) }
    {}

    inline Value::DateTime::DateTime(std::string_view str)
    {
        auto opt = sdk::DateTimeOffset::fromString(str);
        if (!opt.has_value())
        {
            throw std::invalid_argument{ "Invalid DateTime format" };
        }
        value = opt.value();
    }

    inline Value::DateTime::DateTime(sdk::DateTimeOffset dto)
        : value{ std::move(dto) }
    {}

    inline Value::Value(String s)
        : m_value{ std::move(s.value) }
    {}

    inline Value::Value(Integer i)
        : m_value{ i.value }
    {}

    inline Value::Value(Boolean b)
        : m_value{ b.value }
    {}

    inline Value::Value(const Decimal& d)
        : m_value{ d.value }
    {}

    inline Value::Value(const DateTime& dt)
        : m_value{ dt.value }
    {}

    inline std::optional<std::string_view> Value::string() const noexcept
    {
        if (auto* ptr = std::get_if<std::string>(&m_value))
        {
            return *ptr;
        }
        return std::nullopt;
    }

    inline std::optional<bool> Value::boolean() const noexcept
    {
        if (auto* ptr = std::get_if<bool>(&m_value))
        {
            return *ptr;
        }
        return std::nullopt;
    }

    inline std::optional<std::int64_t> Value::integer() const noexcept
    {
        if (auto* ptr = std::get_if<std::int64_t>(&m_value))
        {
            return *ptr;
        }
        return std::nullopt;
    }

    inline std::optional<sdk::Decimal> Value::decimal() const noexcept
    {
        if (auto* ptr = std::get_if<sdk::Decimal>(&m_value))
        {
            return *ptr;
        }
        return std::nullopt;
    }

    inline std::optional<sdk::DateTimeOffset> Value::dateTime() const noexcept
    {
        if (auto* ptr = std::get_if<sdk::DateTimeOffset>(&m_value))
        {
            return *ptr;
        }
        return std::nullopt;
    }

    inline std::string Value::toString() const
    {
        return std::visit(
            [](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return arg ? "true" : "false";
                }
                else if constexpr (std::is_same_v<T, std::int64_t>)
                {
                    return std::to_string(arg);
                }
                else if constexpr (std::is_same_v<T, sdk::Decimal>)
                {
                    return arg.toString();
                }
                else if constexpr (std::is_same_v<T, sdk::DateTimeOffset>)
                {
                    return arg.toString();
                }
            },
            m_value);
    }

    inline Value::Type Value::type() const noexcept
    {
        return static_cast<Type>(m_value.index());
    }

    template <typename FDecimal, typename FInteger, typename FBoolean, typename FString, typename FDateTime>
    inline auto Value::match(
        FDecimal&& onDecimal, FInteger&& onInteger, FBoolean&& onBoolean, FString&& onString, FDateTime&& onDateTime)
        const
    {
        return std::visit(
            [&](auto&& value) -> decltype(auto) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, sdk::Decimal>)
                {
                    return onDecimal(value);
                }
                else if constexpr (std::is_same_v<T, std::int64_t>)
                {
                    return onInteger(value);
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return onBoolean(value);
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return onString(value);
                }
                else if constexpr (std::is_same_v<T, sdk::DateTimeOffset>)
                {
                    return onDateTime(value);
                }
            },
            m_value);
    }

    inline FormatDataType::FormatDataType(std::string type, std::string description)
        : type{ std::move(type) },
          description{ std::move(description) }
    {}

    template <typename FDecimal, typename FInteger, typename FBoolean, typename FString, typename FDateTime>
    inline auto FormatDataType::match(
        std::string_view value,
        FDecimal&& onDecimal,
        FInteger&& onInteger,
        FBoolean&& onBoolean,
        FString&& onString,
        FDateTime&& onDateTime) const
    {
        auto result = validate(value);
        if (!result || !result.value())
        {
            throw std::invalid_argument{
                "Validation failed: " + (result.errors().empty() ? std::string{ "unknown error" } : result.errors()[0])
            };
        }

        return result.value()->match(
            std::forward<FDecimal>(onDecimal),
            std::forward<FInteger>(onInteger),
            std::forward<FBoolean>(onBoolean),
            std::forward<FString>(onString),
            std::forward<FDateTime>(onDateTime));
    }

    FormatDataTypes::FormatDataTypes(std::vector<FormatDataType> values)
        : m_values(std::move(values))
    {}

    inline FormatDataTypes::ConstIterator FormatDataTypes::begin() const noexcept
    {
        return m_values.begin();
    }

    inline FormatDataTypes::ConstIterator FormatDataTypes::end() const noexcept
    {
        return m_values.end();
    }

    inline size_t FormatDataTypes::size() const noexcept
    {
        return m_values.size();
    }

    inline bool FormatDataTypes::isEmpty() const noexcept
    {
        return m_values.empty();
    }
} // namespace dnv::vista::sdk::transport
