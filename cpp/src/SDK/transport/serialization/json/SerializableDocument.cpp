#include "dnv/vista/sdk/transport/serialization/json/SerializableDocument.h"

#include <algorithm>

namespace dnv::vista::sdk::transport::serialization::json
{
    SerializableDocument::SerializableDocument() noexcept
        : m_value{ nullptr }
    {}

    SerializableDocument::SerializableDocument(bool value) noexcept
        : m_value{ value }
    {}

    SerializableDocument::SerializableDocument(std::int64_t value) noexcept
        : m_value{ value }
    {}

    SerializableDocument::SerializableDocument(double value) noexcept
        : m_value{ value }
    {}

    SerializableDocument::SerializableDocument(std::string value)
        : m_value{ std::move(value) }
    {}

    SerializableDocument::SerializableDocument(std::string_view value)
        : m_value{ std::string{ value } }
    {}

    SerializableDocument::SerializableDocument(Array value)
        : m_value{ std::move(value) }
    {}

    SerializableDocument::SerializableDocument(Object value)
        : m_value{ std::move(value) }
    {}

    bool SerializableDocument::operator==(const SerializableDocument& other) const noexcept
    {
        return m_value == other.m_value;
    }

    SerializableDocument SerializableDocument::object() noexcept
    {
        return SerializableDocument{ Object{} };
    }

    SerializableDocument SerializableDocument::array() noexcept
    {
        return SerializableDocument{ Array{} };
    }

    SerializableDocument::Kind SerializableDocument::kind() const noexcept
    {
        switch (m_value.index())
        {
            case 0:
                return Kind::Null;
            case 1:
                return Kind::Boolean;
            case 2:
                return Kind::Integer;
            case 3:
                return Kind::Double;
            case 4:
                return Kind::String;
            case 5:
                return Kind::Array;
            default:
                return Kind::Object;
        }
    }

    bool SerializableDocument::isNull() const noexcept
    {
        return std::holds_alternative<std::nullptr_t>(m_value);
    }

    bool SerializableDocument::isBoolean() const noexcept
    {
        return std::holds_alternative<bool>(m_value);
    }

    bool SerializableDocument::isInteger() const noexcept
    {
        return std::holds_alternative<std::int64_t>(m_value);
    }

    bool SerializableDocument::isDouble() const noexcept
    {
        return std::holds_alternative<double>(m_value);
    }

    bool SerializableDocument::isString() const noexcept
    {
        return std::holds_alternative<std::string>(m_value);
    }

    bool SerializableDocument::isArray() const noexcept
    {
        return std::holds_alternative<Array>(m_value);
    }

    bool SerializableDocument::isObject() const noexcept
    {
        return std::holds_alternative<Object>(m_value);
    }

    bool SerializableDocument::asBoolean() const
    {
        return std::get<bool>(m_value);
    }

    std::int64_t SerializableDocument::asInteger() const
    {
        return std::get<std::int64_t>(m_value);
    }

    double SerializableDocument::asDouble() const
    {
        return std::get<double>(m_value);
    }

    const std::string& SerializableDocument::asString() const
    {
        return std::get<std::string>(m_value);
    }

    const SerializableDocument::Array& SerializableDocument::asArray() const
    {
        return std::get<Array>(m_value);
    }

    const SerializableDocument::Object& SerializableDocument::asObject() const
    {
        return std::get<Object>(m_value);
    }

    SerializableDocument::Array& SerializableDocument::asArray()
    {
        return std::get<Array>(m_value);
    }

    SerializableDocument::Object& SerializableDocument::asObject()
    {
        return std::get<Object>(m_value);
    }

    const SerializableDocument* SerializableDocument::find(std::string_view key) const noexcept
    {
        if (!isObject())
        {
            return nullptr;
        }

        const auto& obj = std::get<Object>(m_value);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& kv) { return kv.first == key; });

        return it == obj.end() ? nullptr : &it->second;
    }

    SerializableDocument* SerializableDocument::find(std::string_view key) noexcept
    {
        if (!isObject())
        {
            return nullptr;
        }

        auto& obj = std::get<Object>(m_value);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& kv) { return kv.first == key; });

        return it == obj.end() ? nullptr : &it->second;
    }

    bool SerializableDocument::contains(std::string_view key) const noexcept
    {
        return find(key) != nullptr;
    }

    void SerializableDocument::set(std::string_view key, SerializableDocument value)
    {
        if (!isObject())
        {
            m_value = Object{};
        }

        auto& obj = std::get<Object>(m_value);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& kv) { return kv.first == key; });

        if (it != obj.end())
        {
            it->second = std::move(value);
        }
        else
        {
            obj.emplace_back(std::string{ key }, std::move(value));
        }
    }

    void SerializableDocument::push_back(SerializableDocument value)
    {
        if (!isArray())
        {
            m_value = Array{};
        }

        std::get<Array>(m_value).push_back(std::move(value));
    }
} // namespace dnv::vista::sdk::transport::serialization::json
