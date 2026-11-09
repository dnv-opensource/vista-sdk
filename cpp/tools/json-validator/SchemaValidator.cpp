#include "SchemaValidator.h"

#include "dnv/vista/sdk/types/datetime/DateTimeOffset.h"

#include <algorithm>
#include <cmath>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace dnv::vista::sdk::json
{
    namespace
    {
        constexpr std::string_view KEY_SCHEMA = "$schema";
        constexpr std::string_view KEY_REF = "$ref";
        constexpr std::string_view KEY_ANCHOR = "$anchor";
        // constexpr std::string_view KEY_DEFS = "$defs";
        // constexpr std::string_view KEY_DEFINITIONS = "definitions";

        constexpr std::string_view KEY_TITLE = "title";
        constexpr std::string_view KEY_DESCRIPTION = "description";

        constexpr std::string_view KEY_TYPE = "type";
        constexpr std::string_view KEY_ENUM = "enum";
        constexpr std::string_view KEY_CONST = "const";

        constexpr std::string_view KEY_PROPERTIES = "properties";
        constexpr std::string_view KEY_PATTERN_PROPERTIES = "patternProperties";
        constexpr std::string_view KEY_ADDITIONAL_PROPERTIES = "additionalProperties";
        constexpr std::string_view KEY_REQUIRED = "required";
        constexpr std::string_view KEY_MIN_PROPERTIES = "minProperties";
        constexpr std::string_view KEY_MAX_PROPERTIES = "maxProperties";
        constexpr std::string_view KEY_DEPENDENT_SCHEMAS = "dependentSchemas";
        constexpr std::string_view KEY_DEPENDENT_REQUIRED = "dependentRequired";

        constexpr std::string_view KEY_ITEMS = "items";
        constexpr std::string_view KEY_PREFIX_ITEMS = "prefixItems";
        constexpr std::string_view KEY_MIN_ITEMS = "minItems";
        constexpr std::string_view KEY_MAX_ITEMS = "maxItems";
        constexpr std::string_view KEY_UNIQUE_ITEMS = "uniqueItems";
        constexpr std::string_view KEY_CONTAINS = "contains";
        constexpr std::string_view KEY_MIN_CONTAINS = "minContains";
        constexpr std::string_view KEY_MAX_CONTAINS = "maxContains";

        constexpr std::string_view KEY_MIN_LENGTH = "minLength";
        constexpr std::string_view KEY_MAX_LENGTH = "maxLength";
        constexpr std::string_view KEY_PATTERN = "pattern";
        constexpr std::string_view KEY_FORMAT = "format";
        constexpr std::string_view FORMAT_DATE_TIME = "date-time";

        constexpr std::string_view KEY_MINIMUM = "minimum";
        constexpr std::string_view KEY_MAXIMUM = "maximum";
        constexpr std::string_view KEY_EXCLUSIVE_MINIMUM = "exclusiveMinimum";
        constexpr std::string_view KEY_EXCLUSIVE_MAXIMUM = "exclusiveMaximum";
        constexpr std::string_view KEY_MULTIPLE_OF = "multipleOf";

        constexpr std::string_view KEY_ALL_OF = "allOf";
        constexpr std::string_view KEY_ANY_OF = "anyOf";
        constexpr std::string_view KEY_ONE_OF = "oneOf";
        constexpr std::string_view KEY_NOT = "not";
        constexpr std::string_view KEY_IF = "if";
        constexpr std::string_view KEY_THEN = "then";
        constexpr std::string_view KEY_ELSE = "else";

        bool isNumeric(const Document& node) noexcept
        {
            return node.isRoot<int64_t>() || node.isRoot<uint64_t>() || node.isRoot<double>();
        }

        double asDouble(const Document& node) noexcept
        {
            if (auto v = node.root<double>())
            {
                return *v;
            }
            if (auto v = node.root<int64_t>())
            {
                return static_cast<double>(*v);
            }
            if (auto v = node.root<uint64_t>())
            {
                return static_cast<double>(*v);
            }
            return 0.0;
        }

        std::string numberToString(double value)
        {
            if (value == std::floor(value) && std::abs(value) < 1e15)
            {
                return std::to_string(static_cast<int64_t>(value));
            }
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }

        std::string describeType(const Document& node) noexcept
        {
            switch (node.type())
            {
                case Type::Null:
                    return "null";
                case Type::Boolean:
                    return "boolean";
                case Type::Integer:
                case Type::UnsignedInteger:
                    return "integer";
                case Type::Double:
                    return "number";
                case Type::String:
                    return "string";
                case Type::Array:
                    return "array";
                case Type::Object:
                    return "object";
            }
            return "unknown";
        }

        bool matchesJsonType(const Document& node, std::string_view expectedType) noexcept
        {
            switch (node.type())
            {
                case Type::Null:
                    return expectedType == "null";
                case Type::Boolean:
                    return expectedType == "boolean";
                case Type::Integer:
                case Type::UnsignedInteger:
                    return expectedType == "integer" || expectedType == "number";
                case Type::Double:
                    return expectedType == "number";
                case Type::String:
                    return expectedType == "string";
                case Type::Array:
                    return expectedType == "array";
                case Type::Object:
                    return expectedType == "object";
            }
            return false;
        }

        std::string appendSegment(std::string_view path, std::string_view segment)
        {
            std::string result{ path };
            result += '/';
            result += segment;
            return result;
        }

        std::string appendIndex(std::string_view path, size_t index)
        {
            return appendSegment(path, std::to_string(index));
        }
    } // namespace

    ValidationError::ValidationError(const ErrorEntry& entry)
        : m_error{ entry }
    {}

    ValidationError::ValidationError(
        std::string path,
        std::string message,
        std::string constraint,
        std::string expectedValue,
        std::string actualValue)
        : m_error{ std::move(path),
                   std::move(message),
                   std::move(constraint),
                   std::move(expectedValue),
                   std::move(actualValue) }
    {}

    std::string ValidationError::toString() const
    {
        std::ostringstream oss;
        oss << "Validation error at path '" << m_error.path << "': " << m_error.message;

        if (!m_error.constraint.empty())
        {
            oss << " (constraint: " << m_error.constraint;
            if (!m_error.expectedValue.empty())
            {
                oss << ", expected: " << m_error.expectedValue;
            }
            if (!m_error.actualValue.empty())
            {
                oss << ", actual: " << m_error.actualValue;
            }
            oss << ")";
        }

        return oss.str();
    }

    ValidationResult::ValidationResult(std::vector<ValidationError> errors)
        : m_errors{ std::move(errors) }
    {}

    const ValidationError& ValidationResult::error(size_t index) const
    {
        if (index >= m_errors.size())
        {
            throw std::out_of_range{ "ValidationResult error index out of range" };
        }
        return m_errors[index];
    }

    std::string ValidationResult::errorSummary() const
    {
        if (m_errors.empty())
        {
            return "No validation errors";
        }

        std::ostringstream oss;
        oss << "Validation failed with " << m_errors.size() << " error(s):\n";

        for (size_t i = 0; i < m_errors.size(); ++i)
        {
            oss << "  " << (i + 1) << ". " << m_errors[i].toString() << "\n";
        }

        return oss.str();
    }

    void ValidationResult::addError(const ValidationError::ErrorEntry& entry)
    {
        m_errors.emplace_back(entry);
    }

    void ValidationResult::addError(const ValidationError& error)
    {
        m_errors.push_back(error);
    }

    void ValidationResult::addError(
        std::string_view path,
        std::string_view message,
        std::string_view constraint,
        std::string_view expectedValue,
        std::string_view actualValue)
    {
        m_errors.emplace_back(
            std::string{ path },
            std::string{ message },
            std::string{ constraint },
            std::string{ expectedValue },
            std::string{ actualValue });
    }

    SchemaValidator::SchemaValidator()
        : m_schema{},
          m_schemaLoaded{ false },
          m_strictMode{ false },
          m_maxDepth{ DEFAULT_MAX_DEPTH },
          m_schemaDraft{ SchemaDraft::Unknown }
    {}

    SchemaValidator::SchemaValidator(const Document& schema)
        : SchemaValidator(schema, Options{})
    {}

    SchemaValidator::SchemaValidator(const Document& schema, const Options& options)
        : m_schema{ schema },
          m_schemaLoaded{ true },
          m_strictMode{ options.strictMode },
          m_maxDepth{ options.maxDepth },
          m_schemaDraft{ SchemaDraft::Unknown }
    {
        detectDraft();
        buildAnchorIndex();
    }

    SchemaValidator::SchemaValidator(const SchemaValidator& other)
        : m_schema{ other.m_schema },
          m_schemaLoaded{ other.m_schemaLoaded },
          m_strictMode{ other.m_strictMode },
          m_maxDepth{ other.m_maxDepth },
          m_schemaDraft{ other.m_schemaDraft },
          m_anchorIndex{ other.m_anchorIndex }
    {}

    SchemaValidator::SchemaValidator(SchemaValidator&& other) noexcept
        : m_schema{ std::move(other.m_schema) },
          m_schemaLoaded{ other.m_schemaLoaded },
          m_strictMode{ other.m_strictMode },
          m_maxDepth{ other.m_maxDepth },
          m_schemaDraft{ other.m_schemaDraft },
          m_refCache{ std::move(other.m_refCache) },
          m_anchorIndex{ std::move(other.m_anchorIndex) }
    {
        other.m_schemaLoaded = false;
        other.m_strictMode = false;
        other.m_maxDepth = 0;
        other.m_schemaDraft = SchemaDraft::Unknown;
    }

    SchemaValidator::~SchemaValidator() = default;

    SchemaValidator& SchemaValidator::operator=(const SchemaValidator& other)
    {
        if (this != &other)
        {
            m_schema = other.m_schema;
            m_schemaLoaded = other.m_schemaLoaded;
            m_strictMode = other.m_strictMode;
            m_maxDepth = other.m_maxDepth;
            m_schemaDraft = other.m_schemaDraft;
            m_refCache.clear();
            m_anchorIndex = other.m_anchorIndex;
        }
        return *this;
    }

    SchemaValidator& SchemaValidator::operator=(SchemaValidator&& other) noexcept
    {
        if (this != &other)
        {
            m_schema = std::move(other.m_schema);
            m_schemaLoaded = other.m_schemaLoaded;
            m_strictMode = other.m_strictMode;
            m_maxDepth = other.m_maxDepth;
            m_schemaDraft = other.m_schemaDraft;
            m_refCache = std::move(other.m_refCache);
            m_anchorIndex = std::move(other.m_anchorIndex);

            other.m_schemaLoaded = false;
            other.m_strictMode = false;
            other.m_maxDepth = 0;
            other.m_schemaDraft = SchemaDraft::Unknown;
        }
        return *this;
    }

    bool SchemaValidator::load(const Document& schema)
    {
        if (!schema.isRoot<Object>() && !schema.isRoot<bool>())
        {
            return false;
        }

        m_schema = schema;
        m_schemaLoaded = true;
        m_refCache.clear();
        detectDraft();
        buildAnchorIndex();
        return true;
    }

    bool SchemaValidator::load(std::string_view schemaJson)
    {
        auto maybeSchema = Document::fromString(schemaJson);
        if (!maybeSchema.has_value())
        {
            return false;
        }

        return load(maybeSchema.value());
    }

    bool SchemaValidator::hasSchema() const
    {
        return m_schemaLoaded;
    }

    void SchemaValidator::clear()
    {
        m_schema = Document{};
        m_schemaLoaded = false;
        m_refCache.clear();
        m_anchorIndex.clear();
        m_schemaDraft = SchemaDraft::Unknown;
    }

    Document SchemaValidator::schema() const
    {
        return m_schema;
    }

    ValidationResult SchemaValidator::validate(const Document& document) const
    {
        if (!m_schemaLoaded)
        {
            throw std::runtime_error{ "No schema loaded for validation" };
        }

        ValidationResult result;
        validateNode(document, m_schema, "", result, 0);
        return result;
    }

    std::string SchemaValidator::version() const
    {
        if (!m_schemaLoaded)
        {
            return {};
        }

        if (auto* node = m_schema.find(KEY_SCHEMA))
        {
            return node->root<std::string>().value_or("");
        }
        return {};
    }

    SchemaDraft SchemaValidator::draft() const
    {
        if (!m_schemaLoaded)
        {
            return SchemaDraft::Unknown;
        }

        return m_schemaDraft;
    }

    std::string SchemaValidator::draftString() const
    {
        if (!m_schemaLoaded)
        {
            return {};
        }

        return draftToString();
    }

    std::string SchemaValidator::title() const
    {
        if (!m_schemaLoaded)
        {
            return {};
        }

        if (auto* node = m_schema.find(KEY_TITLE))
        {
            return node->root<std::string>().value_or("");
        }
        return {};
    }

    std::string SchemaValidator::description() const
    {
        if (!m_schemaLoaded)
        {
            return {};
        }

        if (auto* node = m_schema.find(KEY_DESCRIPTION))
        {
            return node->root<std::string>().value_or("");
        }
        return {};
    }

    void SchemaValidator::validateNode(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        if (m_maxDepth > 0 && currentDepth > m_maxDepth)
        {
            result.addError(
                path,
                "Maximum validation depth exceeded (possible circular $ref)",
                "maxDepth",
                std::to_string(m_maxDepth),
                std::to_string(currentDepth));
            return;
        }

        // Boolean schemas: `true` accepts everything, `false` rejects everything
        if (auto boolSchema = schema.root<bool>())
        {
            if (!*boolSchema)
            {
                result.addError(path, "Value rejected by boolean schema 'false'", "boolean-schema");
            }
            return;
        }

        if (!schema.isRoot<Object>())
        {
            return;
        }

        if (auto* refNode = schema.find(KEY_REF))
        {
            auto refOpt = refNode->root<std::string>();
            if (refOpt.has_value())
            {
                if (!referenceExists(*refOpt))
                {
                    result.addError(
                        path, "Could not resolve reference: " + *refOpt, std::string{ KEY_REF }, *refOpt, "unresolved");
                    return;
                }
                Document resolvedSchema = resolveReference(*refOpt);
                validateNode(document, resolvedSchema, path, result, currentDepth + 1);
                return;
            }
        }

        validateType(document, schema, path, result);
        validateEnum(document, schema, path, result);
        validateConst(document, schema, path, result);

        validateAllOf(document, schema, path, result, currentDepth);
        validateAnyOf(document, schema, path, result, currentDepth);
        validateOneOf(document, schema, path, result, currentDepth);
        validateNot(document, schema, path, result, currentDepth);
        validateIfThenElse(document, schema, path, result, currentDepth);

        switch (document.type())
        {
            case Type::Object:
                validateObject(document, schema, path, result, currentDepth);
                break;
            case Type::Array:
                validateArray(document, schema, path, result, currentDepth);
                break;
            case Type::String:
                validateStringConstraints(document, schema, path, result);
                break;
            case Type::Integer:
            case Type::UnsignedInteger:
            case Type::Double:
                validateNumericConstraints(document, schema, path, result);
                break;
            default:
                break;
        }
    }

    void SchemaValidator::validateType(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        auto* typeNode = schema.find(KEY_TYPE);
        if (!typeNode)
        {
            return;
        }

        // `type` may be a single string or an array of strings
        if (auto singleType = typeNode->root<std::string>())
        {
            if (!matchesJsonType(document, *singleType))
            {
                result.addError(path, "Type mismatch", std::string{ KEY_TYPE }, *singleType, describeType(document));
            }
            return;
        }

        if (typeNode->isRoot<Array>())
        {
            bool matched = false;
            std::string expectedList;
            for (size_t i = 0; i < typeNode->size(); ++i)
            {
                const Document& entry = typeNode->at(i);
                auto entryType = entry.root<std::string>();
                if (!entryType.has_value())
                {
                    continue;
                }
                if (!expectedList.empty())
                {
                    expectedList += ", ";
                }
                expectedList += *entryType;

                if (matchesJsonType(document, *entryType))
                {
                    matched = true;
                }
            }

            if (!matched)
            {
                result.addError(path, "Type mismatch", std::string{ KEY_TYPE }, expectedList, describeType(document));
            }
        }
    }

    void SchemaValidator::validateRequired(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        auto* requiredNode = schema.find(KEY_REQUIRED);
        if (!requiredNode || !requiredNode->isRoot<Array>())
        {
            return;
        }

        for (size_t i = 0; i < requiredNode->size(); ++i)
        {
            const Document& entry = requiredNode->at(i);
            auto propertyName = entry.root<std::string>();
            if (!propertyName.has_value())
            {
                continue;
            }

            if (!document.contains(*propertyName))
            {
                result.addError(
                    path,
                    "Missing required property: " + *propertyName,
                    std::string{ KEY_REQUIRED },
                    *propertyName,
                    "missing");
            }
        }
    }

    void SchemaValidator::validateProperties(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        auto* propertiesNode = schema.find(KEY_PROPERTIES);
        if (!propertiesNode || !propertiesNode->isRoot<Object>())
        {
            return;
        }

        for (auto it = propertiesNode->objectBegin(); it != propertiesNode->objectEnd(); ++it)
        {
            const std::string& propertyName = it.key();
            if (!document.contains(propertyName))
            {
                continue;
            }

            const Document& propertyValue = document.at(propertyName);
            const Document& propertySchema = it.value();
            validateNode(propertyValue, propertySchema, appendSegment(path, propertyName), result, currentDepth + 1);
        }
    }

    void SchemaValidator::validateNumericConstraints(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        if (!isNumeric(document))
        {
            return;
        }

        const double value = asDouble(document);

        if (auto* node = schema.find(KEY_MINIMUM); node && isNumeric(*node))
        {
            const double minimum = asDouble(*node);
            if (value < minimum)
            {
                result.addError(
                    path,
                    "Value is less than minimum",
                    std::string{ KEY_MINIMUM },
                    numberToString(minimum),
                    numberToString(value));
            }
        }

        if (auto* node = schema.find(KEY_EXCLUSIVE_MINIMUM); node && isNumeric(*node))
        {
            const double minimum = asDouble(*node);
            if (value <= minimum)
            {
                result.addError(
                    path,
                    "Value is not greater than exclusive minimum",
                    std::string{ KEY_EXCLUSIVE_MINIMUM },
                    numberToString(minimum),
                    numberToString(value));
            }
        }

        if (auto* node = schema.find(KEY_MAXIMUM); node && isNumeric(*node))
        {
            const double maximum = asDouble(*node);
            if (value > maximum)
            {
                result.addError(
                    path,
                    "Value exceeds maximum",
                    std::string{ KEY_MAXIMUM },
                    numberToString(maximum),
                    numberToString(value));
            }
        }

        if (auto* node = schema.find(KEY_EXCLUSIVE_MAXIMUM); node && isNumeric(*node))
        {
            const double maximum = asDouble(*node);
            if (value >= maximum)
            {
                result.addError(
                    path,
                    "Value is not less than exclusive maximum",
                    std::string{ KEY_EXCLUSIVE_MAXIMUM },
                    numberToString(maximum),
                    numberToString(value));
            }
        }

        if (auto* node = schema.find(KEY_MULTIPLE_OF); node && isNumeric(*node))
        {
            const double divisor = asDouble(*node);
            if (divisor > 0.0)
            {
                const double quotient = value / divisor;
                const double nearest = std::round(quotient);
                constexpr double epsilon = 1e-9;
                if (std::abs(quotient - nearest) > epsilon)
                {
                    result.addError(
                        path,
                        "Value is not a multiple of " + numberToString(divisor),
                        std::string{ KEY_MULTIPLE_OF },
                        numberToString(divisor),
                        numberToString(value));
                }
            }
        }
    }

    void SchemaValidator::validateStringConstraints(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        auto strValue = document.root<std::string>();
        if (!strValue.has_value())
        {
            return;
        }

        // Length constraints are measured in Unicode code points, approximate via UTF-8
        // continuation-byte skipping since the DOM stores raw UTF-8 std::string
        auto codePointLength = [](std::string_view s) -> size_t {
            size_t count = 0;
            for (unsigned char c : s)
            {
                if ((c & 0xC0) != 0x80)
                {
                    ++count;
                }
            }
            return count;
        };

        const size_t length = codePointLength(*strValue);

        if (auto* node = schema.find(KEY_MIN_LENGTH))
        {
            if (auto minLength = node->root<int64_t>(); minLength && length < static_cast<size_t>(*minLength))
            {
                result.addError(
                    path,
                    "String is shorter than minLength",
                    std::string{ KEY_MIN_LENGTH },
                    std::to_string(*minLength),
                    std::to_string(length));
            }
        }

        if (auto* node = schema.find(KEY_MAX_LENGTH))
        {
            if (auto maxLength = node->root<int64_t>(); maxLength && length > static_cast<size_t>(*maxLength))
            {
                result.addError(
                    path,
                    "String exceeds maxLength",
                    std::string{ KEY_MAX_LENGTH },
                    std::to_string(*maxLength),
                    std::to_string(length));
            }
        }

        if (auto* node = schema.find(KEY_PATTERN))
        {
            if (auto pattern = node->root<std::string>())
            {
                try
                {
                    std::regex re{ *pattern, std::regex::ECMAScript };
                    if (!std::regex_search(*strValue, re))
                    {
                        result.addError(
                            path, "String does not match pattern", std::string{ KEY_PATTERN }, *pattern, *strValue);
                    }
                }
                catch (const std::regex_error&)
                {
                    result.addError(path, "Invalid regex pattern in schema", std::string{ KEY_PATTERN }, *pattern, "");
                }
            }
        }

        if (auto* node = schema.find(KEY_FORMAT))
        {
            if (auto format = node->root<std::string>(); format && *format == FORMAT_DATE_TIME)
            {
                if (!DateTimeOffset::fromString(*strValue).has_value())
                {
                    result.addError(
                        path, "String is not a valid date-time", std::string{ KEY_FORMAT }, *format, *strValue);
                }
            }
        }
    }

    void SchemaValidator::validateEnum(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        auto* enumNode = schema.find(KEY_ENUM);
        if (!enumNode || !enumNode->isRoot<Array>())
        {
            return;
        }

        for (size_t i = 0; i < enumNode->size(); ++i)
        {
            if (enumNode->at(i) == document)
            {
                return;
            }
        }

        result.addError(path, "Value is not one of the allowed enum values", std::string{ KEY_ENUM }, "", "");
    }

    void SchemaValidator::validateConst(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        auto* constNode = schema.find(KEY_CONST);
        if (!constNode)
        {
            return;
        }

        if (!(document == *constNode))
        {
            result.addError(path, "Value does not match const", std::string{ KEY_CONST }, "", "");
        }
    }

    void SchemaValidator::validateAdditionalProperties(
        const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const
    {
        auto* additionalNode = schema.find(KEY_ADDITIONAL_PROPERTIES);
        if (!document.isRoot<Object>())
        {
            return;
        }

        std::set<std::string_view> declared;
        if (auto* propertiesNode = schema.find(KEY_PROPERTIES); propertiesNode && propertiesNode->isRoot<Object>())
        {
            for (auto it = propertiesNode->objectBegin(); it != propertiesNode->objectEnd(); ++it)
            {
                declared.insert(it.key());
            }
        }

        std::vector<std::regex> compiledPatterns;
        if (auto* patternPropsNode = schema.find(KEY_PATTERN_PROPERTIES);
            patternPropsNode && patternPropsNode->isRoot<Object>())
        {
            for (auto it = patternPropsNode->objectBegin(); it != patternPropsNode->objectEnd(); ++it)
            {
                try
                {
                    compiledPatterns.emplace_back(it.key(), std::regex::ECMAScript);
                }
                catch (const std::regex_error&)
                {
                    // Ignore malformed patternProperties keys
                }
            }
        }

        const bool additionalAllowed = !additionalNode || additionalNode->root<bool>().value_or(true);
        const Document* additionalSchema =
            (additionalNode && additionalNode->isRoot<Object>()) ? additionalNode : nullptr;

        for (auto it = document.objectBegin(); it != document.objectEnd(); ++it)
        {
            const std::string& key = it.key();
            if (declared.contains(key))
            {
                continue;
            }

            bool matchesPattern =
                std::any_of(compiledPatterns.begin(), compiledPatterns.end(), [&key](const std::regex& re) {
                    return std::regex_search(key, re);
                });
            if (matchesPattern)
            {
                continue;
            }

            if (additionalSchema)
            {
                validateNode(it.value(), *additionalSchema, appendSegment(path, key), result, 0);
                continue;
            }

            if (!additionalAllowed || m_strictMode)
            {
                result.addError(
                    appendSegment(path, key),
                    "Additional property not allowed: " + key,
                    std::string{ KEY_ADDITIONAL_PROPERTIES },
                    "",
                    key);
            }
        }
    }

    void SchemaValidator::validateObject(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        validateRequired(document, schema, path, result);
        validateProperties(document, schema, path, result, currentDepth);
        validateAdditionalProperties(document, schema, path, result);

        if (auto* node = schema.find(KEY_MIN_PROPERTIES))
        {
            if (auto minProps = node->root<int64_t>(); minProps && document.size() < static_cast<size_t>(*minProps))
            {
                result.addError(
                    path,
                    "Object has fewer properties than minProperties",
                    std::string{ KEY_MIN_PROPERTIES },
                    std::to_string(*minProps),
                    std::to_string(document.size()));
            }
        }

        if (auto* node = schema.find(KEY_MAX_PROPERTIES))
        {
            if (auto maxProps = node->root<int64_t>(); maxProps && document.size() > static_cast<size_t>(*maxProps))
            {
                result.addError(
                    path,
                    "Object has more properties than maxProperties",
                    std::string{ KEY_MAX_PROPERTIES },
                    std::to_string(*maxProps),
                    std::to_string(document.size()));
            }
        }

        if (auto* node = schema.find(KEY_DEPENDENT_REQUIRED); node && node->isRoot<Object>())
        {
            for (auto it = node->objectBegin(); it != node->objectEnd(); ++it)
            {
                if (!document.contains(it.key()))
                {
                    continue;
                }
                if (!it.value().isRoot<Array>())
                {
                    continue;
                }
                const Document& requiredList = it.value();
                for (size_t i = 0; i < requiredList.size(); ++i)
                {
                    auto requiredName = requiredList.at(i).root<std::string>();
                    if (requiredName.has_value() && !document.contains(*requiredName))
                    {
                        result.addError(
                            path,
                            "Missing dependent required property: " + *requiredName,
                            std::string{ KEY_DEPENDENT_REQUIRED },
                            *requiredName,
                            "missing");
                    }
                }
            }
        }

        if (auto* node = schema.find(KEY_DEPENDENT_SCHEMAS); node && node->isRoot<Object>())
        {
            for (auto it = node->objectBegin(); it != node->objectEnd(); ++it)
            {
                if (document.contains(it.key()))
                {
                    validateNode(document, it.value(), path, result, currentDepth + 1);
                }
            }
        }
    }

    void SchemaValidator::validateArray(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        if (auto* node = schema.find(KEY_MIN_ITEMS))
        {
            if (auto minItems = node->root<int64_t>(); minItems && document.size() < static_cast<size_t>(*minItems))
            {
                result.addError(
                    path,
                    "Array has fewer items than minItems",
                    std::string{ KEY_MIN_ITEMS },
                    std::to_string(*minItems),
                    std::to_string(document.size()));
            }
        }

        if (auto* node = schema.find(KEY_MAX_ITEMS))
        {
            if (auto maxItems = node->root<int64_t>(); maxItems && document.size() > static_cast<size_t>(*maxItems))
            {
                result.addError(
                    path,
                    "Array has more items than maxItems",
                    std::string{ KEY_MAX_ITEMS },
                    std::to_string(*maxItems),
                    std::to_string(document.size()));
            }
        }

        if (auto* node = schema.find(KEY_UNIQUE_ITEMS))
        {
            if (auto unique = node->root<bool>(); unique && *unique)
            {
                for (size_t i = 0; i < document.size(); ++i)
                {
                    for (size_t j = i + 1; j < document.size(); ++j)
                    {
                        if (document.at(i) == document.at(j))
                        {
                            result.addError(
                                path,
                                "Array items are not unique",
                                std::string{ KEY_UNIQUE_ITEMS },
                                "true",
                                "duplicate at indices " + std::to_string(i) + "," + std::to_string(j));
                            goto uniqueCheckDone;
                        }
                    }
                }
            }
        }
    uniqueCheckDone:;

        // prefixItems (2020-12 tuple validation), falling back to items as array (draft-07 style tuples)
        size_t tupleCount = 0;
        const Document* tupleSchema = nullptr;
        if (auto* prefixNode = schema.find(KEY_PREFIX_ITEMS); prefixNode && prefixNode->isRoot<Array>())
        {
            tupleSchema = prefixNode;
            tupleCount = prefixNode->size();
        }
        else if (auto* itemsNode = schema.find(KEY_ITEMS); itemsNode && itemsNode->isRoot<Array>())
        {
            tupleSchema = itemsNode;
            tupleCount = itemsNode->size();
        }

        size_t validatedCount = 0;
        if (tupleSchema)
        {
            for (size_t i = 0; i < document.size() && i < tupleCount; ++i)
            {
                validateNode(document.at(i), tupleSchema->at(i), appendIndex(path, i), result, currentDepth + 1);
            }
            validatedCount = std::min(document.size(), tupleCount);
        }

        // items as a single schema applies to remaining elements (or all, if no tuple form was used)
        if (auto* itemsNode = schema.find(KEY_ITEMS); itemsNode && !itemsNode->isRoot<Array>())
        {
            for (size_t i = validatedCount; i < document.size(); ++i)
            {
                validateNode(document.at(i), *itemsNode, appendIndex(path, i), result, currentDepth + 1);
            }
        }

        if (auto* containsNode = schema.find(KEY_CONTAINS))
        {
            size_t matchCount = 0;
            for (size_t i = 0; i < document.size(); ++i)
            {
                ValidationResult subResult;
                validateNode(document.at(i), *containsNode, appendIndex(path, i), subResult, currentDepth + 1);
                if (subResult.isValid())
                {
                    ++matchCount;
                }
            }

            size_t minContains = 1;
            if (auto* minNode = schema.find(KEY_MIN_CONTAINS))
            {
                if (auto v = minNode->root<int64_t>())
                {
                    minContains = static_cast<size_t>(*v);
                }
            }

            if (matchCount < minContains)
            {
                result.addError(
                    path,
                    "Array does not contain enough matching items",
                    std::string{ KEY_CONTAINS },
                    std::to_string(minContains),
                    std::to_string(matchCount));
            }

            if (auto* maxNode = schema.find(KEY_MAX_CONTAINS))
            {
                if (auto v = maxNode->root<int64_t>(); v && matchCount > static_cast<size_t>(*v))
                {
                    result.addError(
                        path,
                        "Array contains more matching items than maxContains",
                        std::string{ KEY_MAX_CONTAINS },
                        std::to_string(*v),
                        std::to_string(matchCount));
                }
            }
        }
    }

    void SchemaValidator::validateAllOf(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        auto* node = schema.find(KEY_ALL_OF);
        if (!node || !node->isRoot<Array>())
        {
            return;
        }

        for (size_t i = 0; i < node->size(); ++i)
        {
            validateNode(document, node->at(i), path, result, currentDepth + 1);
        }
    }

    void SchemaValidator::validateAnyOf(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        auto* node = schema.find(KEY_ANY_OF);
        if (!node || !node->isRoot<Array>())
        {
            return;
        }

        for (size_t i = 0; i < node->size(); ++i)
        {
            ValidationResult subResult;
            validateNode(document, node->at(i), path, subResult, currentDepth + 1);
            if (subResult.isValid())
            {
                return;
            }
        }

        result.addError(path, "Value does not match any schema in anyOf", std::string{ KEY_ANY_OF }, "", "");
    }

    void SchemaValidator::validateOneOf(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        auto* node = schema.find(KEY_ONE_OF);
        if (!node || !node->isRoot<Array>())
        {
            return;
        }

        size_t matchCount = 0;
        for (size_t i = 0; i < node->size(); ++i)
        {
            ValidationResult subResult;
            validateNode(document, node->at(i), path, subResult, currentDepth + 1);
            if (subResult.isValid())
            {
                ++matchCount;
            }
        }

        if (matchCount != 1)
        {
            result.addError(
                path,
                "Value must match exactly one schema in oneOf",
                std::string{ KEY_ONE_OF },
                "1",
                std::to_string(matchCount));
        }
    }

    void SchemaValidator::validateNot(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        auto* node = schema.find(KEY_NOT);
        if (!node)
        {
            return;
        }

        ValidationResult subResult;
        validateNode(document, *node, path, subResult, currentDepth + 1);
        if (subResult.isValid())
        {
            result.addError(path, "Value must not match schema in 'not'", std::string{ KEY_NOT }, "", "");
        }
    }

    void SchemaValidator::validateIfThenElse(
        const Document& document,
        const Document& schema,
        std::string_view path,
        ValidationResult& result,
        size_t currentDepth) const
    {
        auto* ifNode = schema.find(KEY_IF);
        if (!ifNode)
        {
            return;
        }

        ValidationResult ifResult;
        validateNode(document, *ifNode, path, ifResult, currentDepth + 1);

        if (ifResult.isValid())
        {
            if (auto* thenNode = schema.find(KEY_THEN))
            {
                validateNode(document, *thenNode, path, result, currentDepth + 1);
            }
        }
        else
        {
            if (auto* elseNode = schema.find(KEY_ELSE))
            {
                validateNode(document, *elseNode, path, result, currentDepth + 1);
            }
        }
    }

    bool SchemaValidator::referenceExists(std::string_view reference) const noexcept
    {
        try
        {
            resolveReference(reference);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    Document SchemaValidator::resolveReference(std::string_view reference) const
    {
        auto cacheKey = std::string{ reference };
        if (auto it = m_refCache.find(cacheKey); it != m_refCache.end())
        {
            return it->second;
        }

        // Only local (same-document) references are supported: "#/a/b/0" or "#anchorName"
        if (reference.empty() || reference[0] != '#')
        {
            throw std::runtime_error{ "Unsupported external $ref: " + std::string{ reference } };
        }

        std::string_view pointer = reference.substr(1);

        if (pointer.empty())
        {
            m_refCache.emplace(cacheKey, m_schema);
            return m_schema;
        }

        // "#anchorName" form (no leading slash): resolve via anchor index
        if (pointer[0] != '/')
        {
            auto anchorIt = m_anchorIndex.find(std::string{ pointer });
            if (anchorIt == m_anchorIndex.end())
            {
                throw std::runtime_error{ "Unknown $anchor: " + std::string{ pointer } };
            }
            pointer = anchorIt->second;
        }
        else
        {
            pointer = pointer.substr(1); // drop leading '/'
        }

        const Document* current = &m_schema;
        size_t start = 0;
        while (start <= pointer.size())
        {
            size_t slash = pointer.find('/', start);
            std::string_view rawSegment =
                slash == std::string_view::npos ? pointer.substr(start) : pointer.substr(start, slash - start);

            if (!rawSegment.empty())
            {
                // JSON Pointer unescaping: ~1 -> /, ~0 -> ~
                std::string segment{ rawSegment };
                size_t pos = 0;
                while ((pos = segment.find("~1", pos)) != std::string::npos)
                {
                    segment.replace(pos, 2, "/");
                    ++pos;
                }
                pos = 0;
                while ((pos = segment.find("~0", pos)) != std::string::npos)
                {
                    segment.replace(pos, 2, "~");
                    ++pos;
                }

                if (current->isRoot<Object>())
                {
                    const Document* next = current->find(segment);
                    if (!next)
                    {
                        throw std::runtime_error{ "Reference path not found: " + std::string{ reference } };
                    }
                    current = next;
                }
                else if (current->isRoot<Array>())
                {
                    size_t index = 0;
                    for (char c : segment)
                    {
                        if (c < '0' || c > '9')
                        {
                            throw std::runtime_error{ "Invalid array index in $ref: " + std::string{ reference } };
                        }
                    }
                    index = static_cast<size_t>(std::stoull(segment));
                    if (index >= current->size())
                    {
                        throw std::runtime_error{ "Array index out of range in $ref: " + std::string{ reference } };
                    }
                    current = &current->at(index);
                }
                else
                {
                    throw std::runtime_error{ "Cannot traverse into scalar for $ref: " + std::string{ reference } };
                }
            }

            if (slash == std::string_view::npos)
            {
                break;
            }
            start = slash + 1;
        }

        Document resolved = *current;
        m_refCache.emplace(cacheKey, resolved);
        return resolved;
    }

    Document SchemaValidator::extractSubDocument(const Document& sourceDocument, std::string_view path) const
    {
        const Document* current = &sourceDocument;
        std::string_view remaining = path;
        if (!remaining.empty() && remaining[0] == '/')
        {
            remaining = remaining.substr(1);
        }

        while (!remaining.empty())
        {
            size_t slash = remaining.find('/');
            std::string_view segment = slash == std::string_view::npos ? remaining : remaining.substr(0, slash);

            if (current->isRoot<Object>())
            {
                const Document* next = current->find(segment);
                if (!next)
                {
                    return Document{ nullptr };
                }
                current = next;
            }
            else if (current->isRoot<Array>())
            {
                bool numeric = !segment.empty() &&
                               std::all_of(segment.begin(), segment.end(), [](char c) { return c >= '0' && c <= '9'; });
                if (!numeric)
                {
                    return Document{ nullptr };
                }
                size_t index = static_cast<size_t>(std::stoull(std::string{ segment }));
                if (index >= current->size())
                {
                    return Document{ nullptr };
                }
                current = &current->at(index);
            }
            else
            {
                return Document{ nullptr };
            }

            if (slash == std::string_view::npos)
            {
                break;
            }
            remaining = remaining.substr(slash + 1);
        }

        return *current;
    }

    void SchemaValidator::buildAnchorIndex()
    {
        m_anchorIndex.clear();
        if (!m_schema.isRoot<Object>())
        {
            return;
        }
        scanForAnchors(m_schema, "");
    }

    void SchemaValidator::scanForAnchors(const Document& schema, const std::string& currentPath)
    {
        if (schema.isRoot<Object>())
        {
            if (auto* anchorNode = schema.find(KEY_ANCHOR))
            {
                if (auto anchorName = anchorNode->root<std::string>())
                {
                    m_anchorIndex.emplace(*anchorName, currentPath);
                }
            }

            for (auto it = schema.objectBegin(); it != schema.objectEnd(); ++it)
            {
                std::string childPath = currentPath.empty() ? it.key() : currentPath + "/" + it.key();
                scanForAnchors(it.value(), childPath);
            }
        }
        else if (schema.isRoot<Array>())
        {
            for (size_t i = 0; i < schema.size(); ++i)
            {
                std::string childPath = currentPath + "/" + std::to_string(i);
                scanForAnchors(schema.at(i), childPath);
            }
        }
    }

    void SchemaValidator::detectDraft()
    {
        m_schemaDraft = SchemaDraft::Unknown;

        auto* schemaNode = m_schema.find(KEY_SCHEMA);
        if (!schemaNode)
        {
            return;
        }

        auto schemaUri = schemaNode->root<std::string>();
        if (!schemaUri.has_value())
        {
            return;
        }

        if (schemaUri->find("2020-12") != std::string::npos)
        {
            m_schemaDraft = SchemaDraft::Draft202012;
        }
        else if (schemaUri->find("2019-09") != std::string::npos)
        {
            m_schemaDraft = SchemaDraft::Draft201909;
        }
        else if (schemaUri->find("draft-07") != std::string::npos)
        {
            m_schemaDraft = SchemaDraft::Draft07;
        }
        else if (schemaUri->find("draft-06") != std::string::npos)
        {
            m_schemaDraft = SchemaDraft::Draft06;
        }
        else if (schemaUri->find("draft-04") != std::string::npos)
        {
            m_schemaDraft = SchemaDraft::Draft04;
        }
    }

    std::string SchemaValidator::draftToString() const noexcept
    {
        switch (m_schemaDraft)
        {
            case SchemaDraft::Draft04:
                return "04";
            case SchemaDraft::Draft06:
                return "06";
            case SchemaDraft::Draft07:
                return "07";
            case SchemaDraft::Draft201909:
                return "2019-09";
            case SchemaDraft::Draft202012:
                return "2020-12";
            case SchemaDraft::Unknown:
            default:
                return "";
        }
    }
} // namespace dnv::vista::sdk::json
