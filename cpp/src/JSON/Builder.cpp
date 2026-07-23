#include "Builder.h"

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    #include <emmintrin.h>
    #define DNV_VISTA_SDK_JSON_BUILDER_USE_SIMD 1
#else
    #define DNV_VISTA_SDK_JSON_BUILDER_USE_SIMD 0
#endif

namespace dnv::vista::sdk::json
{
    Builder::Builder(StringBuilder& buffer, Options options)
        : m_buffer{ buffer },
          m_indent{ options.indent },
          m_currentIndent{ 0 },
          m_escapeNonAscii{ options.escapeNonAscii }
    {}

    Builder& Builder::writeStartObject()
    {
        writeCommaIfNeeded();
        m_buffer += '{';
        m_contextStack.push_back({ true, true, false });
        if (m_indent > 0)
        {
            m_currentIndent += m_indent;
        }
        return *this;
    }

    Builder& Builder::writeEndObject()
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "writeEndObject called without matching writeStartObject" };
        }
        if (m_indent > 0)
        {
            m_currentIndent -= m_indent;
            if (!m_contextStack.back().empty)
            {
                writeNewlineAndIndent();
            }
        }
        m_contextStack.pop_back();
        m_buffer += '}';
        return *this;
    }

    Builder& Builder::writeStartArray()
    {
        writeCommaIfNeeded();
        m_buffer += '[';
        m_contextStack.push_back({ false, true, false });
        if (m_indent > 0)
        {
            m_currentIndent += m_indent;
        }
        return *this;
    }

    Builder& Builder::writeEndArray()
    {
        if (m_contextStack.empty() || m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "writeEndArray called without matching writeStartArray" };
        }
        if (m_indent > 0)
        {
            m_currentIndent -= m_indent;
            if (!m_contextStack.back().empty)
            {
                writeNewlineAndIndent();
            }
        }
        m_contextStack.pop_back();
        m_buffer += ']';
        return *this;
    }

    Builder& Builder::writeKey(std::string_view key)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "writeKey can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeString(key);
        if (m_indent > 0)
        {
            m_buffer += ": ";
        }
        else
        {
            m_buffer += ':';
        }
        m_contextStack.back().expectingValue = true;
        return *this;
    }

    Builder& Builder::writeTrustedKey(std::string_view key)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "writeTrustedKey can only be called inside an object" };
        }
        writeCommaIfNeeded();
        writeTrustedKeyImpl(key);
        return *this;
    }

    Builder& Builder::write(std::string_view key, std::nullptr_t)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        m_buffer += "null";
        return *this;
    }

    Builder& Builder::write(std::string_view key, bool value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        m_buffer += (value ? "true" : "false");
        return *this;
    }

    Builder& Builder::write(std::string_view key, int value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeInt(static_cast<int64_t>(value));
        return *this;
    }

    Builder& Builder::write(std::string_view key, unsigned int value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeUInt(static_cast<uint64_t>(value));
        return *this;
    }

    Builder& Builder::write(std::string_view key, int64_t value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeInt(value);
        return *this;
    }

    Builder& Builder::write(std::string_view key, uint64_t value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeUInt(value);
        return *this;
    }

    Builder& Builder::write(std::string_view key, float value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeDouble(static_cast<double>(value));
        return *this;
    }

    Builder& Builder::write(std::string_view key, double value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeDouble(value);
        return *this;
    }

    Builder& Builder::write(std::string_view key, std::string_view value)
    {
        if (m_contextStack.empty() || !m_contextStack.back().isObject)
        {
            throw std::runtime_error{ "write can only be called inside an object" };
        }
        writeKeyImpl(key);
        writeCommaIfNeeded();
        writeString(value);
        return *this;
    }

    Builder& Builder::write(std::string_view key, const char* value)
    {
        return write(key, std::string_view{ value });
    }

    Builder& Builder::write(std::string_view key, const std::string& value)
    {
        return write(key, std::string_view{ value });
    }

    Builder& Builder::write(std::string_view key, const Document& value)
    {
        writeKeyImpl(key);
        writeDocument(value);
        return *this;
    }

    Builder& Builder::write(std::nullptr_t)
    {
        writeCommaIfNeeded();
        m_buffer += "null";
        return *this;
    }

    Builder& Builder::write(bool value)
    {
        writeCommaIfNeeded();
        m_buffer += (value ? "true" : "false");
        return *this;
    }

    Builder& Builder::write(int value)
    {
        writeCommaIfNeeded();
        writeInt(static_cast<int64_t>(value));
        return *this;
    }

    Builder& Builder::write(unsigned int value)
    {
        writeCommaIfNeeded();
        writeUInt(static_cast<uint64_t>(value));
        return *this;
    }

    Builder& Builder::write(int64_t value)
    {
        writeCommaIfNeeded();
        writeInt(value);
        return *this;
    }

    Builder& Builder::write(uint64_t value)
    {
        writeCommaIfNeeded();
        writeUInt(value);
        return *this;
    }

    Builder& Builder::write(float value)
    {
        writeCommaIfNeeded();
        writeDouble(static_cast<double>(value));
        return *this;
    }

    Builder& Builder::write(double value)
    {
        writeCommaIfNeeded();
        writeDouble(value);
        return *this;
    }

    Builder& Builder::write(std::string_view value)
    {
        writeCommaIfNeeded();
        writeString(value);
        return *this;
    }

    Builder& Builder::write(const char* value)
    {
        return write(std::string_view{ value });
    }

    Builder& Builder::write(const std::string& value)
    {
        return write(std::string_view{ value });
    }

    Builder& Builder::write(const Document& value)
    {
        writeCommaIfNeeded();
        writeDocument(value);
        return *this;
    }

    std::string Builder::toString()
    {
        std::string result = m_buffer.toString();
        m_buffer.clear();
        m_contextStack.clear();
        m_currentIndent = 0;
        return result;
    }

    Builder& Builder::reset()
    {
        m_buffer.clear();
        m_contextStack.clear();
        m_currentIndent = 0;
        return *this;
    }

    size_t Builder::size() const noexcept
    {
        return m_buffer.size();
    }

    bool Builder::isEmpty() const noexcept
    {
        return m_buffer.isEmpty();
    }

    Builder& Builder::reserve(size_t cap)
    {
        m_buffer.reserve(cap);
        return *this;
    }

    size_t Builder::capacity() const noexcept
    {
        return m_buffer.capacity();
    }

    bool Builder::isValid() const noexcept
    {
        return m_contextStack.empty();
    }

    void Builder::writeKeyImpl(std::string_view key)
    {
        writeCommaIfNeeded();
        writeString(key);
        if (m_indent > 0)
        {
            m_buffer += ": ";
        }
        else
        {
            m_buffer += ':';
        }
        m_contextStack.back().expectingValue = true;
    }

    void Builder::writeTrustedKeyImpl(std::string_view key)
    {
        writeStringRaw(key);
        if (m_indent > 0)
        {
            m_buffer += ": ";
        }
        else
        {
            m_buffer += ':';
        }
        m_contextStack.back().expectingValue = true;
    }

    void Builder::writeStringRaw(std::string_view str)
    {
        m_buffer += '"';
        m_buffer += str;
        m_buffer += '"';
    }

    void Builder::writeInt(int64_t value)
    {
        m_buffer.append(value);
    }

    void Builder::writeUInt(uint64_t value)
    {
        m_buffer.append(value);
    }

    void Builder::writeDouble(double value)
    {
        m_buffer.append(value);
    }

    void Builder::writeString(std::string_view str)
    {
        m_buffer += '"';

        if (m_escapeNonAscii)
        {
            static constexpr char hex[] = "0123456789abcdef";
            for (size_t i = 0; i < str.size();)
            {
                unsigned char c = static_cast<unsigned char>(str[i]);
                if (c < 0x80)
                {
                    if (c == '"' || c == '\\' || c < 0x20)
                    {
                        m_buffer += '\\';
                        switch (c)
                        {
                            case '"':
                                m_buffer += '"';
                                break;
                            case '\\':
                                m_buffer += '\\';
                                break;
                            case '\b':
                                m_buffer += 'b';
                                break;
                            case '\f':
                                m_buffer += 'f';
                                break;
                            case '\n':
                                m_buffer += 'n';
                                break;
                            case '\r':
                                m_buffer += 'r';
                                break;
                            case '\t':
                                m_buffer += 't';
                                break;
                            default:
                                m_buffer += 'u';
                                m_buffer += '0';
                                m_buffer += '0';
                                m_buffer += hex[(c >> 4) & 0xF];
                                m_buffer += hex[c & 0xF];
                                break;
                        }
                    }
                    else
                    {
                        m_buffer += static_cast<char>(c);
                    }
                    ++i;
                }
                else
                {
                    // Decode UTF-8 code point and emit \uXXXX or surrogate pair
                    uint32_t codePoint = 0;
                    size_t seqLen = 1;
                    if ((c & 0xE0) == 0xC0 && i + 1 < str.size())
                    {
                        codePoint = (c & 0x1F);
                        codePoint = (codePoint << 6) | (static_cast<unsigned char>(str[i + 1]) & 0x3F);
                        seqLen = 2;
                    }
                    else if ((c & 0xF0) == 0xE0 && i + 2 < str.size())
                    {
                        codePoint = (c & 0x0F);
                        codePoint = (codePoint << 6) | (static_cast<unsigned char>(str[i + 1]) & 0x3F);
                        codePoint = (codePoint << 6) | (static_cast<unsigned char>(str[i + 2]) & 0x3F);
                        seqLen = 3;
                    }
                    else if ((c & 0xF8) == 0xF0 && i + 3 < str.size())
                    {
                        codePoint = (c & 0x07);
                        codePoint = (codePoint << 6) | (static_cast<unsigned char>(str[i + 1]) & 0x3F);
                        codePoint = (codePoint << 6) | (static_cast<unsigned char>(str[i + 2]) & 0x3F);
                        codePoint = (codePoint << 6) | (static_cast<unsigned char>(str[i + 3]) & 0x3F);
                        seqLen = 4;
                    }
                    else
                    {
                        // Invalid UTF-8 byte: escape as \u00XX
                        m_buffer += "\\u00";
                        m_buffer += hex[(c >> 4) & 0xF];
                        m_buffer += hex[c & 0xF];
                        ++i;
                        continue;
                    }
                    if (codePoint >= 0x10000)
                    {
                        // Surrogate pair
                        codePoint -= 0x10000;
                        uint32_t high = 0xD800 + ((codePoint >> 10) & 0x3FF);
                        uint32_t low = 0xDC00 + (codePoint & 0x3FF);
                        m_buffer += "\\u";
                        m_buffer += hex[(high >> 12) & 0xF];
                        m_buffer += hex[(high >> 8) & 0xF];
                        m_buffer += hex[(high >> 4) & 0xF];
                        m_buffer += hex[high & 0xF];
                        m_buffer += "\\u";
                        m_buffer += hex[(low >> 12) & 0xF];
                        m_buffer += hex[(low >> 8) & 0xF];
                        m_buffer += hex[(low >> 4) & 0xF];
                        m_buffer += hex[low & 0xF];
                    }
                    else
                    {
                        m_buffer += "\\u";
                        m_buffer += hex[(codePoint >> 12) & 0xF];
                        m_buffer += hex[(codePoint >> 8) & 0xF];
                        m_buffer += hex[(codePoint >> 4) & 0xF];
                        m_buffer += hex[codePoint & 0xF];
                    }
                    i += seqLen;
                }
            }
            m_buffer += '"';
            return;
        }

#if DNV_VISTA_SDK_JSON_BUILDER_USE_SIMD
        static constexpr char hex[] = "0123456789abcdef";
        size_t lastPos = 0;
        size_t i = 0;

        const __m128i quote = _mm_set1_epi8('"');
        const __m128i backslash = _mm_set1_epi8('\\');
        const __m128i control_threshold = _mm_set1_epi8(0x20);
        const __m128i sign_flip = _mm_set1_epi8(static_cast<char>(0x80));

        while (i + 16 <= str.size())
        {
            __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str.data() + i));
            __m128i cmp_quote = _mm_cmpeq_epi8(chunk, quote);
            __m128i cmp_backslash = _mm_cmpeq_epi8(chunk, backslash);
            __m128i chunk_unsigned = _mm_xor_si128(chunk, sign_flip);
            __m128i threshold_unsigned = _mm_xor_si128(control_threshold, sign_flip);
            __m128i cmp_control = _mm_cmplt_epi8(chunk_unsigned, threshold_unsigned);
            __m128i needs_escape = _mm_or_si128(_mm_or_si128(cmp_quote, cmp_backslash), cmp_control);
            int mask = _mm_movemask_epi8(needs_escape);

            if (mask == 0)
            {
                i += 16;
            }
            else
            {
                if (i > lastPos)
                {
                    m_buffer.append(std::string_view{ str.data() + lastPos, i - lastPos });
                    lastPos = i;
                }
                for (size_t j = 0; j < 16 && i + j < str.size(); ++j)
                {
                    if (mask & (1 << j))
                    {
                        if (i + j > lastPos)
                        {
                            m_buffer.append(std::string_view{ str.data() + lastPos, (i + j) - lastPos });
                        }
                        unsigned char c = static_cast<unsigned char>(str[i + j]);
                        m_buffer += '\\';
                        switch (c)
                        {
                            case '"':
                                m_buffer += '"';
                                break;
                            case '\\':
                                m_buffer += '\\';
                                break;
                            case '\b':
                                m_buffer += 'b';
                                break;
                            case '\f':
                                m_buffer += 'f';
                                break;
                            case '\n':
                                m_buffer += 'n';
                                break;
                            case '\r':
                                m_buffer += 'r';
                                break;
                            case '\t':
                                m_buffer += 't';
                                break;
                            default:
                                m_buffer += 'u';
                                m_buffer += '0';
                                m_buffer += '0';
                                m_buffer += hex[(c >> 4) & 0xF];
                                m_buffer += hex[c & 0xF];
                                break;
                        }
                        lastPos = i + j + 1;
                    }
                }
                i += 16;
            }
        }

        for (; i < str.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(str[i]);
            if (c == '"' || c == '\\' || c < 0x20)
            {
                if (i > lastPos)
                {
                    m_buffer.append(std::string_view{ str.data() + lastPos, i - lastPos });
                }
                m_buffer += '\\';
                switch (c)
                {
                    case '"':
                        m_buffer += '"';
                        break;
                    case '\\':
                        m_buffer += '\\';
                        break;
                    case '\b':
                        m_buffer += 'b';
                        break;
                    case '\f':
                        m_buffer += 'f';
                        break;
                    case '\n':
                        m_buffer += 'n';
                        break;
                    case '\r':
                        m_buffer += 'r';
                        break;
                    case '\t':
                        m_buffer += 't';
                        break;
                    default:
                        m_buffer += "u00";
                        m_buffer += hex[(c >> 4) & 0xF];
                        m_buffer += hex[c & 0xF];
                        break;
                }
                lastPos = i + 1;
            }
        }

        if (lastPos < str.size())
        {
            m_buffer.append(std::string_view{ str.data() + lastPos, str.size() - lastPos });
        }
#else
        static constexpr char hex[] = "0123456789abcdef";
        size_t lastPos = 0;
        for (size_t i = 0; i < str.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(str[i]);
            if (c == '"' || c == '\\' || c < 0x20)
            {
                if (i > lastPos)
                {
                    m_buffer.append(std::string_view{ str.data() + lastPos, i - lastPos });
                }
                m_buffer += '\\';
                switch (c)
                {
                    case '"':
                        m_buffer += '"';
                        break;
                    case '\\':
                        m_buffer += '\\';
                        break;
                    case '\b':
                        m_buffer += 'b';
                        break;
                    case '\f':
                        m_buffer += 'f';
                        break;
                    case '\n':
                        m_buffer += 'n';
                        break;
                    case '\r':
                        m_buffer += 'r';
                        break;
                    case '\t':
                        m_buffer += 't';
                        break;
                    default:
                        m_buffer += "u00";
                        m_buffer += hex[(c >> 4) & 0xF];
                        m_buffer += hex[c & 0xF];
                        break;
                }
                lastPos = i + 1;
            }
        }
        if (lastPos < str.size())
        {
            m_buffer.append(std::string_view{ str.data() + lastPos, str.size() - lastPos });
        }
#endif

        m_buffer += '"';
    }

    void Builder::writeNewlineAndIndent()
    {
        m_buffer += '\n';
        if (m_currentIndent > 0)
        {
            m_buffer.appendRepeated(' ', static_cast<size_t>(m_currentIndent));
        }
    }

    void Builder::writeCommaIfNeeded()
    {
        if (m_contextStack.empty())
        {
            return;
        }
        auto& context = m_contextStack.back();
        if (context.expectingValue)
        {
            context.expectingValue = false;
            context.empty = false;
            return;
        }
        if (!context.empty)
        {
            m_buffer += ',';
            if (m_indent > 0)
            {
                writeNewlineAndIndent();
            }
        }
        else
        {
            context.empty = false;
            if (m_indent > 0)
            {
                writeNewlineAndIndent();
            }
        }
    }

    void Builder::writeDocument(const Document& doc)
    {
        switch (doc.type())
        {
            case Type::Null:
                m_buffer += "null";
                break;
            case Type::Boolean:
                if (auto val = doc.root<bool>())
                {
                    m_buffer += (*val ? "true" : "false");
                }
                break;
            case Type::Integer:
                if (auto val = doc.root<int64_t>())
                {
                    writeInt(*val);
                }
                break;
            case Type::UnsignedInteger:
                if (auto val = doc.root<uint64_t>())
                {
                    writeUInt(*val);
                }
                break;
            case Type::Double:
                if (auto val = doc.root<double>())
                {
                    writeDouble(*val);
                }
                break;
            case Type::String:
                if (auto val = doc.root<std::string>())
                {
                    writeString(*val);
                }
                break;
            case Type::Array:
                if (auto arrRef = doc.rootRef<Array>())
                {
                    writeDocumentArray(arrRef->get());
                }
                break;
            case Type::Object:
                if (auto objRef = doc.rootRef<Object>())
                {
                    writeDocumentObject(objRef->get());
                }
                break;
        }
    }

    void Builder::writeDocumentArray(const Array& array)
    {
        m_buffer += '[';
        m_contextStack.push_back({ false, true, false });
        if (m_indent > 0)
        {
            m_currentIndent += m_indent;
        }
        for (const auto& element : array)
        {
            writeCommaIfNeeded();
            writeDocument(element);
        }
        if (m_indent > 0)
        {
            m_currentIndent -= m_indent;
            if (!m_contextStack.back().empty)
            {
                writeNewlineAndIndent();
            }
        }
        m_buffer += ']';
        m_contextStack.pop_back();
    }

    void Builder::writeDocumentObject(const Object& object)
    {
        m_buffer += '{';
        m_contextStack.push_back({ true, true, false });
        if (m_indent > 0)
        {
            m_currentIndent += m_indent;
        }
        for (const auto& [key, value] : object)
        {
            writeCommaIfNeeded();
            writeString(key);
            if (m_indent > 0)
            {
                m_buffer += ": ";
            }
            else
            {
                m_buffer += ':';
            }
            writeDocument(value);
        }
        if (m_indent > 0)
        {
            m_currentIndent -= m_indent;
            if (!m_contextStack.back().empty)
            {
                writeNewlineAndIndent();
            }
        }
        m_buffer += '}';
        m_contextStack.pop_back();
    }
} // namespace dnv::vista::sdk::json
