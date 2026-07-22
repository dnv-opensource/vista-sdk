#include "Document.h"

#include "Builder.h"
#include "Parser.h"

#include <algorithm>

namespace dnv::vista::sdk::json
{
    Document::Document()
        : m_data{ Object{} }
    {}

    Document::Document(bool value) noexcept
        : m_data{ value }
    {}

    Document::Document(int value) noexcept
        : m_data{ static_cast<int64_t>(value) }
    {}

    Document::Document(unsigned int value) noexcept
        : m_data{ static_cast<uint64_t>(value) }
    {}

    Document::Document(int64_t value) noexcept
        : m_data{ value }
    {}

    Document::Document(uint64_t value) noexcept
        : m_data{ value }
    {}

    Document::Document(double value) noexcept
        : m_data{ value }
    {}

    Document::Document(std::string value)
        : m_data{ std::move(value) }
    {}

    Document::Document(std::string_view value)
        : m_data{ std::string{ value } }
    {}

    Document::Document(const char* value)
        : m_data{ std::string{ value } }
    {}

    Document::Document(std::nullptr_t) noexcept
        : m_data{ nullptr }
    {}

    Document::Document(Array value)
        : m_data{ std::move(value) }
    {}

    Document::Document(Object value)
        : m_data{ std::move(value) }
    {}

    Document& Document::operator=(std::string value)
    {
        m_data = std::move(value);
        return *this;
    }

    Document& Document::operator=(bool value) noexcept
    {
        m_data = value;
        return *this;
    }

    Document& Document::operator=(int64_t value) noexcept
    {
        m_data = value;
        return *this;
    }

    Document& Document::operator=(uint64_t value) noexcept
    {
        m_data = value;
        return *this;
    }

    Document& Document::operator=(double value) noexcept
    {
        m_data = value;
        return *this;
    }

    Document& Document::operator=(std::nullptr_t) noexcept
    {
        m_data = nullptr;
        return *this;
    }

    bool Document::operator==(const Document& other) const noexcept
    {
        return m_data == other.m_data;
    }

    std::strong_ordering Document::operator<=>(const Document& other) const noexcept
    {
        if (type() != other.type())
        {
            return type() <=> other.type();
        }
        switch (type())
        {
            case Type::Null:
                return std::strong_ordering::equal;
            case Type::Boolean:
                return std::get<bool>(m_data) <=> std::get<bool>(other.m_data);
            case Type::Integer:
                return std::get<int64_t>(m_data) <=> std::get<int64_t>(other.m_data);
            case Type::UnsignedInteger:
                return std::get<uint64_t>(m_data) <=> std::get<uint64_t>(other.m_data);
            case Type::Double:
            {
                double a = std::get<double>(m_data);
                double b = std::get<double>(other.m_data);
                if (a < b)
                {
                    return std::strong_ordering::less;
                }
                if (a > b)
                {
                    return std::strong_ordering::greater;
                }
                return std::strong_ordering::equal;
            }
            case Type::String:
                return std::get<std::string>(m_data) <=> std::get<std::string>(other.m_data);
            case Type::Array:
            {
                const auto& a = std::get<Array>(m_data);
                const auto& b = std::get<Array>(other.m_data);
                return std::lexicographical_compare_three_way(
                    a.begin(), a.end(), b.begin(), b.end(), [](const Document& x, const Document& y) {
                        return x <=> y;
                    });
            }
            case Type::Object:
            {
                const auto& a = std::get<Object>(m_data);
                const auto& b = std::get<Object>(other.m_data);
                return std::lexicographical_compare_three_way(
                    a.begin(), a.end(), b.begin(), b.end(), [](const auto& x, const auto& y) {
                        if (auto cmp = x.first <=> y.first; cmp != 0)
                        {
                            return cmp;
                        }
                        return x.second <=> y.second;
                    });
            }
        }
        return std::strong_ordering::equal;
    }

    Document Document::object()
    {
        return Document{ Object{} };
    }

    Document Document::array()
    {
        return Document{ Array{} };
    }

    Type Document::type() const noexcept
    {
        static constexpr Type typeMapping[]{ Type::Null,   Type::Boolean, Type::Integer, Type::UnsignedInteger,
                                             Type::Double, Type::String,  Type::Array,   Type::Object };
        return typeMapping[m_data.index()];
    }

    Document* Document::find(std::string_view key) noexcept
    {
        if (type() != Type::Object)
        {
            return nullptr;
        }
        auto& obj = std::get<Object>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& pair) { return pair.first == key; });
        if (it != obj.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    const Document* Document::find(std::string_view key) const noexcept
    {
        return const_cast<Document*>(this)->find(key);
    }

    void Document::set(std::string key, Document value)
    {
        if (type() != Type::Object)
        {
            m_data = Object{};
        }
        auto& obj = std::get<Object>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [&key](const auto& pair) { return pair.first == key; });
        if (it != obj.end())
        {
            it->second = std::move(value);
        }
        else
        {
            obj.emplace_back(std::move(key), std::move(value));
        }
    }

    bool Document::contains(std::string_view key) const noexcept
    {
        if (type() != Type::Object)
        {
            return false;
        }
        const auto& obj = std::get<Object>(m_data);
        return std::find_if(obj.begin(), obj.end(), [key](const auto& pair) { return pair.first == key; }) != obj.end();
    }

    Document& Document::at(std::string_view key)
    {
        if (type() != Type::Object)
        {
            throw std::runtime_error{ "Not an object" };
        }
        auto& obj = std::get<Object>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& pair) { return pair.first == key; });
        if (it == obj.end())
        {
            throw std::out_of_range{ "Key not found" };
        }
        return it->second;
    }

    const Document& Document::at(std::string_view key) const
    {
        return const_cast<Document*>(this)->at(key);
    }

    Document& Document::at(size_t index)
    {
        if (type() != Type::Array)
        {
            throw std::runtime_error{ "Not an array" };
        }
        auto& arr = std::get<Array>(m_data);
        if (index >= arr.size())
        {
            throw std::out_of_range{ "Array index out of bounds" };
        }
        return arr[index];
    }

    const Document& Document::at(size_t index) const
    {
        return const_cast<Document*>(this)->at(index);
    }

    Document& Document::operator[](std::string_view key)
    {
        if (type() == Type::Null)
        {
            m_data = Object{};
        }
#ifndef NDEBUG
        if (type() != Type::Object)
        {
            throw std::runtime_error{ "operator[]: Cannot use key access on non-Object type" };
        }
#endif
        auto& obj = std::get<Object>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& pair) { return pair.first == key; });
        if (it != obj.end())
        {
            return it->second;
        }
        obj.emplace_back(std::string{ key }, Document{});
        return obj.back().second;
    }

    const Document& Document::operator[](std::string_view key) const
    {
        static const Document null_value{};
        if (type() != Type::Object)
        {
            return null_value;
        }
        const auto& obj = std::get<Object>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& pair) { return pair.first == key; });
        if (it == obj.end())
        {
            return null_value;
        }
        return it->second;
    }

    Document& Document::operator[](size_t index)
    {
        if (type() == Type::Null)
        {
            m_data = Array{};
        }
#ifndef NDEBUG
        if (type() != Type::Array)
        {
            throw std::runtime_error{ "operator[]: Cannot use index access on non-Array type" };
        }
#endif
        auto& arr = std::get<Array>(m_data);
        if (index >= arr.size())
        {
            arr.resize(index + 1, Document{ nullptr });
        }
        return arr[index];
    }

    const Document& Document::operator[](size_t index) const
    {
        static const Document nullValue{ nullptr };
        if (type() != Type::Array)
        {
            return nullValue;
        }
        const auto& arr = std::get<Array>(m_data);
        if (index >= arr.size())
        {
            return nullValue;
        }
        return arr[index];
    }

    void Document::clear()
    {
        if (type() == Type::Object)
        {
            std::get<Object>(m_data).clear();
        }
        else if (type() == Type::Array)
        {
            std::get<Array>(m_data).clear();
        }
    }

    size_t Document::erase(std::string_view key)
    {
        if (type() != Type::Object)
        {
            return 0;
        }
        auto& obj = std::get<Object>(m_data);
        auto it = std::find_if(obj.begin(), obj.end(), [key](const auto& pair) { return pair.first == key; });
        if (it != obj.end())
        {
            obj.erase(it);
            return 1;
        }
        return 0;
    }

    size_t Document::erase(const char* key)
    {
        return erase(std::string_view(key));
    }

    size_t Document::erase(const std::string& key)
    {
        return erase(std::string_view(key));
    }

    size_t Document::size() const noexcept
    {
        if (type() == Type::Array)
        {
            return std::get<Array>(m_data).size();
        }
        if (type() == Type::Object)
        {
            return std::get<Object>(m_data).size();
        }
        return 0;
    }

    bool Document::isEmpty() const noexcept
    {
        switch (type())
        {
            case Type::Array:
                return std::get<Array>(m_data).empty();
            case Type::Object:
                return std::get<Object>(m_data).empty();
            case Type::String:
                return std::get<std::string>(m_data).empty();
            default:
                return false;
        }
    }

    void Document::push_back(Document value)
    {
        if (type() != Type::Array)
        {
            m_data = Array{};
        }
        std::get<Array>(m_data).push_back(std::move(value));
    }

    void Document::reserve(size_t capacity)
    {
        if (type() == Type::Array)
        {
            std::get<Array>(m_data).reserve(capacity);
        }
        else if (type() == Type::Object)
        {
            std::get<Object>(m_data).reserve(capacity);
        }
    }

    size_t Document::capacity() const noexcept
    {
        if (type() == Type::Array)
        {
            return std::get<Array>(m_data).capacity();
        }
        if (type() == Type::Object)
        {
            return std::get<Object>(m_data).capacity();
        }
        return 0;
    }

    Document& Document::front()
    {
        auto& arr = std::get<Array>(m_data);
        if (arr.empty())
        {
            throw std::out_of_range{ "Array is empty" };
        }
        return arr.front();
    }

    const Document& Document::front() const
    {
        return const_cast<Document*>(this)->front();
    }

    Document& Document::back()
    {
        auto& arr = std::get<Array>(m_data);
        if (arr.empty())
        {
            throw std::out_of_range{ "Array is empty" };
        }
        return arr.back();
    }

    const Document& Document::back() const
    {
        return const_cast<Document*>(this)->back();
    }

    auto Document::begin()
    {
        return std::get<Array>(m_data).begin();
    }

    auto Document::end()
    {
        return std::get<Array>(m_data).end();
    }

    auto Document::begin() const
    {
        return std::get<Array>(m_data).begin();
    }

    auto Document::end() const
    {
        return std::get<Array>(m_data).end();
    }

    Document::ObjectIterator::ObjectIterator(MapIterator iter)
        : it{ iter }
    {}

    const std::string& Document::ObjectIterator::key() const
    {
        return it->first;
    }

    const Document& Document::ObjectIterator::value() const
    {
        return it->second;
    }

    Document& Document::ObjectIterator::value()
    {
        return const_cast<Document&>(it->second);
    }

    const Document& Document::ObjectIterator::operator*() const
    {
        return it->second;
    }

    Document& Document::ObjectIterator::operator*()
    {
        return const_cast<Document&>(it->second);
    }

    Document::ObjectIterator& Document::ObjectIterator::operator++()
    {
        ++it;
        return *this;
    }

    bool Document::ObjectIterator::operator==(const ObjectIterator& other) const
    {
        return it == other.it;
    }

    Document::ObjectIterator Document::objectBegin() const
    {
        if (type() != Type::Object)
        {
            static const Object emptyObj;
            return ObjectIterator(emptyObj.end());
        }
        return ObjectIterator(std::get<Object>(m_data).begin());
    }

    Document::ObjectIterator Document::objectEnd() const
    {
        if (type() != Type::Object)
        {
            static const Object emptyObj;
            return ObjectIterator(emptyObj.end());
        }
        return ObjectIterator(std::get<Object>(m_data).end());
    }

    std::string Document::toString(int indent, size_t bufferSize) const
    {
        StringBuilder sb{ bufferSize > 0 ? bufferSize : 4096 };
        Builder builder{ sb, { .indent = indent } };
        builder.write(*this);
        return builder.toString();
    }

    std::optional<Document> Document::fromString(std::string_view jsonStr)
    {
        if (jsonStr.empty())
        {
            return std::nullopt;
        }
        size_t start = 0;
        size_t end = jsonStr.size();
        while (start < end &&
               (jsonStr[start] == ' ' || jsonStr[start] == '\t' || jsonStr[start] == '\n' || jsonStr[start] == '\r'))
        {
            ++start;
        }
        while (end > start && (jsonStr[end - 1] == ' ' || jsonStr[end - 1] == '\t' || jsonStr[end - 1] == '\n' ||
                               jsonStr[end - 1] == '\r'))
        {
            --end;
        }
        if (start >= end)
        {
            return std::nullopt;
        }
        try
        {
            return Parser::parse(jsonStr);
        }
        catch (const std::runtime_error&)
        {
            return std::nullopt;
        }
    }

    bool Document::fromString(std::string_view jsonStr, Document& value)
    {
        auto result = fromString(jsonStr);
        if (result)
        {
            value = std::move(*result);
            return true;
        }
        return false;
    }

    std::optional<Document> Document::fromBytes(const std::vector<uint8_t>& bytes)
    {
        if (bytes.empty())
        {
            return std::nullopt;
        }
        try
        {
            std::string_view jsonStr(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            return Parser::parse(jsonStr);
        }
        catch (const std::runtime_error&)
        {
            return std::nullopt;
        }
    }

    bool Document::fromBytes(const std::vector<uint8_t>& bytes, Document& value)
    {
        auto result = fromBytes(bytes);
        if (result)
        {
            value = std::move(*result);
            return true;
        }
        return false;
    }
} // namespace dnv::vista::sdk::json
