/**
 * @file Parser.h
 * @brief JSON parser
 * @details Converts a JSON string into a Document DOM tree.
 *          Supports UTF-8 BOM, unicode escapes (\uXXXX), surrogate pairs, and all
 *          standard JSON value types. SIMD acceleration (SSE2) is enabled automatically
 *          when the target platform supports it.
 */

#pragma once

#include "Document.h"

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    #include <emmintrin.h>
    #define DNV_VISTA_SDK_JSON_PARSER_USE_SIMD 1
#else
    #define DNV_VISTA_SDK_JSON_PARSER_USE_SIMD 0
#endif

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace dnv::vista::sdk::json
{
    namespace
    {
        constexpr size_t STRING_INITIAL_CAPACITY = 64;
        constexpr size_t SIMD_CHUNK_SIZE = 16;

        inline int countTrailingZeros(unsigned int value)
        {
#if defined(_MSC_VER)
            if (value == 0)
            {
                return 32;
            }
            unsigned long index;
            _BitScanForward(&index, value);
            return static_cast<int>(index);
#elif defined(__GNUC__) || defined(__clang__)
            return __builtin_ctz(value);
#else
            if (value == 0)
            {
                return 32;
            }
            int count = 0;
            while ((value & 1) == 0)
            {
                value >>= 1;
                ++count;
            }
            return count;
#endif
        }

        constexpr uint8_t UTF8_BOM_BYTE1 = 0xEF;
        constexpr uint8_t UTF8_BOM_BYTE2 = 0xBB;
        constexpr uint8_t UTF8_BOM_BYTE3 = 0xBF;
        constexpr size_t UTF8_BOM_LENGTH = 3;

        constexpr uint32_t UTF8_1BYTE_MAX = 0x80;
        constexpr uint32_t UTF8_2BYTE_MAX = 0x800;
        constexpr uint32_t UTF8_3BYTE_MAX = 0x10000;
        constexpr uint32_t UTF8_4BYTE_MAX = 0x110000;

        constexpr uint8_t UTF8_CONTINUATION_BYTE = 0x80;
        constexpr uint8_t UTF8_CONTINUATION_MASK = 0x3F;
        constexpr uint8_t UTF8_2BYTE_LEAD = 0xC0;
        constexpr uint8_t UTF8_3BYTE_LEAD = 0xE0;
        constexpr uint8_t UTF8_4BYTE_LEAD = 0xF0;

        constexpr uint32_t UTF16_HIGH_SURROGATE_START = 0xD800;
        constexpr uint32_t UTF16_HIGH_SURROGATE_END = 0xDBFF;
        constexpr uint32_t UTF16_LOW_SURROGATE_START = 0xDC00;
        constexpr uint32_t UTF16_LOW_SURROGATE_END = 0xDFFF;
        constexpr uint32_t UTF16_SURROGATE_OFFSET = 0x10000;
    } // namespace

    /**
     * @brief JSON text-to-DOM parser
     * @details Not intended for direct use. Prefer Document::fromString() which wraps parse()
     *          and returns std::nullopt on failure instead of throwing.
     */
    class Parser final
    {
    public:
        /**
         * @brief Parses a JSON string into a Document
         * @param json The JSON text to parse (UTF-8, BOM optional)
         * @return The parsed Document
         * @throws std::runtime_error on malformed JSON
         */
        static Document parse(std::string_view json)
        {
            if (json.size() >= UTF8_BOM_LENGTH && static_cast<unsigned char>(json[0]) == UTF8_BOM_BYTE1 &&
                static_cast<unsigned char>(json[1]) == UTF8_BOM_BYTE2 &&
                static_cast<unsigned char>(json[2]) == UTF8_BOM_BYTE3)
            {
                json.remove_prefix(UTF8_BOM_LENGTH);
            }
            Parser parser{ json };
            return parser.parseValue();
        }

    private:
        explicit Parser(std::string_view json)
            : m_json{ json },
              m_pos{ 0 }
        {}

        uint32_t parseSurrogatePair(uint32_t highSurrogate)
        {
            m_pos += 4;
            if (m_pos + 6 >= m_json.size() || m_json[m_pos + 1] != '\\' || m_json[m_pos + 2] != 'u')
            {
                throw std::runtime_error{ "Invalid surrogate pair: missing low surrogate" };
            }
            const char* lowStart = m_json.data() + m_pos + 3;
            unsigned int lowSurrogate = 0;
            auto [lowPtr, lowEc] = std::from_chars(lowStart, lowStart + 4, lowSurrogate, 16);
            if (lowEc != std::errc{} || lowPtr != lowStart + 4)
            {
                throw std::runtime_error{ "Invalid unicode escape in low surrogate" };
            }
            if (lowSurrogate < UTF16_LOW_SURROGATE_START || lowSurrogate > UTF16_LOW_SURROGATE_END)
            {
                throw std::runtime_error{ "Invalid low surrogate value" };
            }
            uint32_t codePoint = ((highSurrogate - UTF16_HIGH_SURROGATE_START) << 10) +
                                 (lowSurrogate - UTF16_LOW_SURROGATE_START) + UTF16_SURROGATE_OFFSET;
            m_pos += 6;
            return codePoint;
        }

        void encodeUtf8(std::string& result, uint32_t codePoint)
        {
            if (codePoint < UTF8_1BYTE_MAX)
            {
                result.push_back(static_cast<char>(codePoint));
            }
            else if (codePoint < UTF8_2BYTE_MAX)
            {
                result.push_back(static_cast<char>(UTF8_2BYTE_LEAD | (codePoint >> 6)));
                result.push_back(static_cast<char>(UTF8_CONTINUATION_BYTE | (codePoint & UTF8_CONTINUATION_MASK)));
            }
            else if (codePoint < UTF8_3BYTE_MAX)
            {
                result.push_back(static_cast<char>(UTF8_3BYTE_LEAD | (codePoint >> 12)));
                result.push_back(
                    static_cast<char>(UTF8_CONTINUATION_BYTE | ((codePoint >> 6) & UTF8_CONTINUATION_MASK)));
                result.push_back(static_cast<char>(UTF8_CONTINUATION_BYTE | (codePoint & UTF8_CONTINUATION_MASK)));
            }
            else if (codePoint < UTF8_4BYTE_MAX)
            {
                result.push_back(static_cast<char>(UTF8_4BYTE_LEAD | (codePoint >> 18)));
                result.push_back(
                    static_cast<char>(UTF8_CONTINUATION_BYTE | ((codePoint >> 12) & UTF8_CONTINUATION_MASK)));
                result.push_back(
                    static_cast<char>(UTF8_CONTINUATION_BYTE | ((codePoint >> 6) & UTF8_CONTINUATION_MASK)));
                result.push_back(static_cast<char>(UTF8_CONTINUATION_BYTE | (codePoint & UTF8_CONTINUATION_MASK)));
            }
            else
            {
                throw std::runtime_error{ "Invalid unicode code point" };
            }
        }

        void parseUnicodeEscape(std::string& result)
        {
            if (m_pos + 4 >= m_json.size())
            {
                throw std::runtime_error{ "Invalid unicode escape" };
            }
            const char* start = m_json.data() + m_pos + 1;
            unsigned int codePoint = 0;
            auto [ptr, ec] = std::from_chars(start, start + 4, codePoint, 16);
            if (ec != std::errc{} || ptr != start + 4)
            {
                throw std::runtime_error{ "Invalid unicode escape" };
            }
            if (codePoint >= UTF16_HIGH_SURROGATE_START && codePoint <= UTF16_HIGH_SURROGATE_END)
            {
                codePoint = parseSurrogatePair(codePoint);
            }
            else if (codePoint >= UTF16_LOW_SURROGATE_START && codePoint <= UTF16_LOW_SURROGATE_END)
            {
                throw std::runtime_error{ "Invalid surrogate pair: unexpected low surrogate" };
            }
            else
            {
                m_pos += 4;
            }
            encodeUtf8(result, codePoint);
        }

        void parseEscapeSequence(std::string& result)
        {
            if (m_pos >= m_json.size())
            {
                throw std::runtime_error{ "Unexpected end in string escape" };
            }
            char escaped = m_json[m_pos];
            switch (escaped)
            {
                case '"':
                    result.push_back('"');
                    break;
                case '\\':
                    result.push_back('\\');
                    break;
                case '/':
                    result.push_back('/');
                    break;
                case 'b':
                    result.push_back('\b');
                    break;
                case 'f':
                    result.push_back('\f');
                    break;
                case 'n':
                    result.push_back('\n');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'u':
                    parseUnicodeEscape(result);
                    break;
                default:
                    throw std::runtime_error{ std::string{ "Invalid escape sequence: \\" } + escaped };
            }
            ++m_pos;
        }

        void skipWhitespace()
        {
#if DNV_VISTA_SDK_JSON_PARSER_USE_SIMD
            const __m128i space_vec = _mm_set1_epi8(' ');
            const __m128i tab_vec = _mm_set1_epi8('\t');
            const __m128i newline_vec = _mm_set1_epi8('\n');
            const __m128i cr_vec = _mm_set1_epi8('\r');

            while (m_pos + SIMD_CHUNK_SIZE <= m_json.size())
            {
                __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(m_json.data() + m_pos));
                __m128i is_space = _mm_cmpeq_epi8(chunk, space_vec);
                __m128i is_tab = _mm_cmpeq_epi8(chunk, tab_vec);
                __m128i is_newline = _mm_cmpeq_epi8(chunk, newline_vec);
                __m128i is_cr = _mm_cmpeq_epi8(chunk, cr_vec);
                __m128i whitespace = _mm_or_si128(_mm_or_si128(is_space, is_tab), _mm_or_si128(is_newline, is_cr));
                int mask = _mm_movemask_epi8(whitespace);
                if (mask == 0xFFFF)
                {
                    m_pos += SIMD_CHUNK_SIZE;
                }
                else
                {
                    int non_ws_mask = ~mask & 0xFFFF;
                    int leading_ws = countTrailingZeros(static_cast<unsigned int>(non_ws_mask));
                    m_pos += leading_ws;
                    break;
                }
            }
#endif
            while (m_pos < m_json.size())
            {
                char c = m_json[m_pos];
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                {
                    ++m_pos;
                }
                else
                {
                    break;
                }
            }
        }

        inline Document parseValue()
        {
            skipWhitespace();
            if (m_pos >= m_json.size())
            {
                throw std::runtime_error{ "Unexpected end of JSON" };
            }
            char c = m_json[m_pos];
            switch (c)
            {
                case 'n':
                    return parseNull();
                case 't':
                case 'f':
                    return parseBool();
                case '"':
                    return parseString();
                case '[':
                    return parseArray();
                case '{':
                    return parseObject();
                case '-':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    return parseNumber();
                default:
                    throw std::runtime_error{ std::string{ "Unexpected character: " } + c };
            }
        }

        inline Document parseNull()
        {
            if (m_json.substr(m_pos, 4) != "null")
            {
                throw std::runtime_error{ "Expected 'null'" };
            }
            m_pos += 4;
            return Document{ nullptr };
        }

        inline Document parseBool()
        {
            if (m_json.substr(m_pos, 4) == "true")
            {
                m_pos += 4;
                return Document{ true };
            }
            else if (m_json.substr(m_pos, 5) == "false")
            {
                m_pos += 5;
                return Document{ false };
            }
            else
            {
                throw std::runtime_error{ "Expected 'true' or 'false'" };
            }
        }

        inline Document parseNumber()
        {
            size_t start = m_pos;
            if (m_pos < m_json.size() && m_json[m_pos] == '-')
            {
                ++m_pos;
            }
            while (m_pos < m_json.size() && m_json[m_pos] >= '0' && m_json[m_pos] <= '9')
            {
                ++m_pos;
            }
            bool isDouble = false;
            if (m_pos < m_json.size() && m_json[m_pos] == '.')
            {
                isDouble = true;
                ++m_pos;
                while (m_pos < m_json.size() && m_json[m_pos] >= '0' && m_json[m_pos] <= '9')
                {
                    ++m_pos;
                }
            }
            if (m_pos < m_json.size() && (m_json[m_pos] == 'e' || m_json[m_pos] == 'E'))
            {
                isDouble = true;
                ++m_pos;
                if (m_pos < m_json.size() && (m_json[m_pos] == '+' || m_json[m_pos] == '-'))
                {
                    ++m_pos;
                }
                while (m_pos < m_json.size() && m_json[m_pos] >= '0' && m_json[m_pos] <= '9')
                {
                    ++m_pos;
                }
            }
            std::string_view numStr = m_json.substr(start, m_pos - start);
            if (isDouble)
            {
                double value = 0.0;

#if defined(__EMSCRIPTEN__)
                char* endPtr = nullptr;
                std::string numStrZ{ numStr };
                value = std::strtod(numStrZ.c_str(), &endPtr);
                if (endPtr != numStrZ.c_str() + numStrZ.size())
                {
                    throw std::runtime_error{ "Invalid number format" };
                }
#else
                auto ec = std::from_chars(numStr.data(), numStr.data() + numStr.size(), value).ec;
                if (ec != std::errc{})
                {
                    throw std::runtime_error{ "Invalid number format" };
                }
#endif
                return Document{ value };
            }
            else
            {
                int64_t intValue = 0;
                auto ec = std::from_chars(numStr.data(), numStr.data() + numStr.size(), intValue).ec;
                if (ec == std::errc{})
                {
                    return Document{ intValue };
                }
                uint64_t uintValue = 0;
                auto ec2 = std::from_chars(numStr.data(), numStr.data() + numStr.size(), uintValue).ec;
                if (ec2 == std::errc{})
                {
                    return Document{ uintValue };
                }
                throw std::runtime_error{ "Invalid number format" };
            }
        }

        inline Document parseString()
        {
            if (m_json[m_pos] != '"')
            {
                throw std::runtime_error{ "Expected '\"'" };
            }
            ++m_pos;
            std::string result;
            result.reserve(STRING_INITIAL_CAPACITY);
#if DNV_VISTA_SDK_JSON_PARSER_USE_SIMD
            const __m128i quote_vec = _mm_set1_epi8('"');
            const __m128i backslash_vec = _mm_set1_epi8('\\');
            while (m_pos + SIMD_CHUNK_SIZE <= m_json.size())
            {
                __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(m_json.data() + m_pos));
                __m128i cmp_quote = _mm_cmpeq_epi8(chunk, quote_vec);
                __m128i cmp_backslash = _mm_cmpeq_epi8(chunk, backslash_vec);
                __m128i special = _mm_or_si128(cmp_quote, cmp_backslash);
                int mask = _mm_movemask_epi8(special);
                if (mask == 0)
                {
                    result.append(m_json.data() + m_pos, SIMD_CHUNK_SIZE);
                    m_pos += SIMD_CHUNK_SIZE;
                }
                else
                {
                    int special_pos = countTrailingZeros(static_cast<unsigned int>(mask));
                    result.append(m_json.data() + m_pos, special_pos);
                    m_pos += special_pos;
                    break;
                }
            }
#endif
            while (m_pos < m_json.size())
            {
                char c = m_json[m_pos];
                if (c == '"')
                {
                    ++m_pos;
                    return Document{ std::move(result) };
                }
                else if (c == '\\')
                {
                    ++m_pos;
                    parseEscapeSequence(result);
                }
                else
                {
                    unsigned char uc = static_cast<unsigned char>(c);
                    if (uc < 0x20)
                    {
                        throw std::runtime_error{ "Unescaped control character in string" };
                    }
                    result.push_back(c);
                    ++m_pos;
                }
            }
            throw std::runtime_error{ "Unterminated string" };
        }

        inline Document parseArray()
        {
            if (m_json[m_pos] != '[')
            {
                throw std::runtime_error{ "Expected '['" };
            }
            ++m_pos;
            Array arr;
            skipWhitespace();
            if (m_pos < m_json.size() && m_json[m_pos] == ']')
            {
                ++m_pos;
                return Document{ std::move(arr) };
            }
            while (true)
            {
                arr.push_back(parseValue());
                skipWhitespace();
                if (m_pos >= m_json.size())
                {
                    throw std::runtime_error{ "Unexpected end of array" };
                }
                if (m_json[m_pos] == ']')
                {
                    ++m_pos;
                    return Document{ std::move(arr) };
                }
                else if (m_json[m_pos] == ',')
                {
                    ++m_pos;
                    skipWhitespace();
                }
                else
                {
                    throw std::runtime_error{ "Expected ',' or ']' in array" };
                }
            }
        }

        inline Document parseObject()
        {
            if (m_json[m_pos] != '{')
            {
                throw std::runtime_error{ "Expected '{'" };
            }
            ++m_pos;
            Object obj;
            skipWhitespace();
            if (m_pos < m_json.size() && m_json[m_pos] == '}')
            {
                ++m_pos;
                return Document{ std::move(obj) };
            }
            while (true)
            {
                skipWhitespace();
                if (m_pos >= m_json.size() || m_json[m_pos] != '"')
                {
                    throw std::runtime_error{ "Expected string key in object" };
                }
                Document keyValue = parseString();
                auto keyOpt = keyValue.root<std::string>();
                if (!keyOpt)
                {
                    throw std::runtime_error{ "Failed to extract string key from parsed value" };
                }
                std::string key = std::move(keyOpt.value());
                skipWhitespace();
                if (m_pos >= m_json.size() || m_json[m_pos] != ':')
                {
                    throw std::runtime_error{ "Expected ':' in object" };
                }
                ++m_pos;
                Document value = parseValue();
                obj.emplace_back(std::move(key), std::move(value));
                skipWhitespace();
                if (m_pos >= m_json.size())
                {
                    throw std::runtime_error{ "Unexpected end of object" };
                }
                if (m_json[m_pos] == '}')
                {
                    ++m_pos;
                    return Document{ std::move(obj) };
                }
                else if (m_json[m_pos] == ',')
                {
                    ++m_pos;
                }
                else
                {
                    throw std::runtime_error{ "Expected ',' or '}' in object" };
                }
            }
        }

        std::string_view m_json;
        size_t m_pos;
    };
} // namespace dnv::vista::sdk::json
