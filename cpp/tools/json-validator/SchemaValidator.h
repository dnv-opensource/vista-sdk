/**
 * @file SchemaValidator.h
 * @brief JSON Schema validation for Document instances
 * @details Provides JSON Schema Draft 2020-12 validation capabilities for verifying Document
 *          structure, types, constraints, and business rules against schema definitions
 *          Supports validation with detailed error reporting and path tracking
 */

#pragma once

#include <JSON/Document.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dnv::vista::sdk::json
{
    /**
     * @brief Represents a single JSON schema validation error
     * @details Contains detailed information about validation failures including
     *          the failed constraint, document path, expected vs actual values,
     *          and human-readable error messages
     */
    class ValidationError final
    {
    public:
        /**
         * @brief Simple error entry structure for convenient ValidationError construction
         * @details Provides aggregate initialization syntax for creating validation errors
         *          with all error details in a single struct. Useful for functional-style
         *          error creation and collection patterns.
         */
        struct ErrorEntry
        {
            std::string path;               ///< JSON path where error occurred
            std::string message;            ///< Human-readable error message
            std::string constraint;         ///< Schema constraint that failed
            std::string expectedValue = {}; ///< Expected value or constraint
            std::string actualValue = {};   ///< Actual value found in document
        };

    public:
        /**
         * @brief Construct validation error from ErrorEntry struct
         * @param entry Error entry with all validation details
         */
        explicit ValidationError(const ErrorEntry& entry);

        /**
         * @brief Construct validation error
         * @param path JSON path where validation failed
         * @param message Human-readable error description
         * @param constraint The schema constraint that failed (e.g., "type", "minLength")
         * @param expectedValue Expected value or constraint (optional)
         * @param actualValue Actual value found in document (optional)
         */
        ValidationError(
            std::string path,
            std::string message,
            std::string constraint,
            std::string expectedValue = {},
            std::string actualValue = {});

        ~ValidationError() = default;

        /**
         * @brief Get the JSON path where validation failed
         * @return Path string (e.g., "/users/0/email")
         */
        inline const std::string& path() const noexcept { return m_error.path; }

        /**
         * @brief Get human-readable error message
         * @return Error description
         */
        inline const std::string& message() const noexcept { return m_error.message; }

        /**
         * @brief Get the constraint type that failed
         * @return Constraint name (e.g., "type", "minLength", "required")
         */
        inline const std::string& constraint() const noexcept { return m_error.constraint; }

        /**
         * @brief Get expected value or constraint
         * @return Expected value string, or empty if not applicable
         */
        inline const std::string& expectedValue() const noexcept { return m_error.expectedValue; }

        /**
         * @brief Get actual value found in document
         * @return Actual value string, or empty if not applicable
         */
        inline const std::string& actualValue() const noexcept { return m_error.actualValue; }

        /**
         * @brief Get formatted error string
         * @return Complete formatted error message with path and details
         */
        std::string toString() const;

    private:
        ErrorEntry m_error;
    };

    /**
     * @brief Result of JSON schema validation operation
     * @details Contains validation success status and error reporting
     *          with all validation failures found during document validation
     */
    class ValidationResult final
    {
    public:
        /**
         * @brief Construct successful validation result
         */
        ValidationResult() = default;

        /**
         * @brief Construct validation result with errors
         * @param errors List of validation errors found
         */
        explicit ValidationResult(std::vector<ValidationError> errors);

        ~ValidationResult() = default;

        /**
         * @brief Check if validation was successful
         * @return True if no validation errors, false otherwise
         */
        inline bool isValid() const noexcept { return m_errors.empty(); }

        /**
         * @brief Check if validation failed
         * @return True if validation errors exist, false otherwise
         */
        inline bool hasErrors() const noexcept { return !m_errors.empty(); }

        /**
         * @brief Get number of validation errors
         * @return Count of validation errors found
         */
        inline size_t errorCount() const noexcept { return m_errors.size(); }

        /**
         * @brief Get all validation errors
         * @return Vector of validation errors
         */
        inline const std::vector<ValidationError>& errors() const noexcept { return m_errors; }

        /**
         * @brief Get validation error by index
         * @param index Error index (0-based)
         * @return Validation error at specified index
         * @throws std::out_of_range if index is invalid
         */
        const ValidationError& error(size_t index) const;

        /**
         * @brief Get formatted error summary
         * @return Multi-line string with all validation errors
         */
        std::string errorSummary() const;

        /**
         * @brief Add validation error from ErrorEntry
         * @param entry The error entry to add
         */
        void addError(const ValidationError::ErrorEntry& entry);

        /**
         * @brief Add validation error to result
         * @param error The validation error to add
         */
        void addError(const ValidationError& error);

        /**
         * @brief Add validation error with details
         * @param path JSON path where validation failed
         * @param message Human-readable error description
         * @param constraint The schema constraint that failed
         * @param expectedValue Expected value or constraint (optional)
         * @param actualValue Actual value found in document (optional)
         */
        void addError(
            std::string_view path,
            std::string_view message,
            std::string_view constraint,
            std::string_view expectedValue = {},
            std::string_view actualValue = {});

    private:
        std::vector<ValidationError> m_errors;
    };

    /**
     * @brief Enumeration of supported JSON Schema draft versions
     * @details Used to identify and track which JSON Schema draft version a schema conforms to
     *          Detection is based on the `$schema` keyword in the schema document
     */
    enum class SchemaDraft
    {
        Unknown,     ///< Schema draft version is not recognized or not specified
        Draft04,     ///< JSON Schema Draft 4
        Draft06,     ///< JSON Schema Draft 6
        Draft07,     ///< JSON Schema Draft 7
        Draft201909, ///< JSON Schema Draft 2019-09
        Draft202012  ///< JSON Schema Draft 2020-12 (current)
    };

    /**
     * @brief JSON Schema validator for Document instances
     * @details Provides JSON Schema Draft 2020-12 validation capabilities
     *          including type checking, constraint validation, object/array validation,
     *          and custom validation rules. Supports schema loading from Documents
     *          and detailed error reporting with path tracking
     * @warning NOT thread-safe. Do not call validate() concurrently on the same instance
     */
    class SchemaValidator final
    {
    public:
        /**
         * @brief Configuration options for validation
         */
        struct Options
        {
            bool strictMode = false; ///< If true, unknown properties cause validation failure
            size_t maxDepth = 64;    ///< Maximum recursion depth (0 = unlimited)
        };

        /**
         * @brief Default constructor - creates empty validator
         * @details Creates validator without schema. Must load schema before validation.
         */
        SchemaValidator();

        /**
         * @brief Construct validator with schema (default options)
         * @param schema JSON Schema document to use for validation
         */
        explicit SchemaValidator(const Document& schema);

        /**
         * @brief Construct validator with schema
         * @param schema JSON Schema document to use for validation
         * @param options Validation options
         */
        SchemaValidator(const Document& schema, const Options& options);

        SchemaValidator(const SchemaValidator& other);
        SchemaValidator(SchemaValidator&& other) noexcept;

        ~SchemaValidator();

        SchemaValidator& operator=(const SchemaValidator& other);
        SchemaValidator& operator=(SchemaValidator&& other) noexcept;

        /**
         * @brief Load JSON schema from Document
         * @param schema JSON Schema document
         * @return True if schema loaded successfully, false on parse error
         */
        bool load(const Document& schema);

        /**
         * @brief Load JSON schema from string
         * @param schemaJson JSON Schema as string
         * @return True if schema loaded successfully, false on parse error
         */
        bool load(std::string_view schemaJson);

        /**
         * @brief Check if validator has valid schema loaded
         * @return True if schema is loaded and valid, false otherwise
         */
        bool hasSchema() const;

        /**
         * @brief Clear loaded schema
         */
        void clear();

        /**
         * @brief Get current schema as Document
         * @return Current schema Document, or empty Document if no schema loaded
         */
        Document schema() const;

        /**
         * @brief Validate Document against loaded schema
         * @param document The JSON document to validate
         * @return ValidationResult with success status and error details
         * @throws std::runtime_error if no schema is loaded
         * @warning NOT thread-safe (modifies internal cache)
         */
        ValidationResult validate(const Document& document) const;

        /**
         * @brief Get schema version/draft URI
         * @return Schema $schema URI string, or empty if not specified
         */
        std::string version() const;

        /**
         * @brief Get detected JSON Schema draft version
         * @return SchemaDraft enumeration value indicating the draft version
         */
        SchemaDraft draft() const;

        /**
         * @brief Get schema draft version as human-readable string
         * @return Draft version string (e.g., "2020-12", "07"), or empty if unknown
         */
        std::string draftString() const;

        /**
         * @brief Get schema title
         * @return Schema title string, or empty if not specified
         */
        std::string title() const;

        /**
         * @brief Get schema description
         * @return Schema description string, or empty if not specified
         */
        std::string description() const;

    private:
        /**
         * @brief Validate a document node against schema node
         * @param document Document to validate
         * @param schema Schema to validate against
         * @param path Current JSON path for error reporting
         * @param result ValidationResult to accumulate errors
         * @param currentDepth Current recursion depth for circular reference protection
         */
        void validateNode(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateType(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateRequired(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateProperties(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateNumericConstraints(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateStringConstraints(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateEnum(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateConst(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateAdditionalProperties(
            const Document& document, const Document& schema, std::string_view path, ValidationResult& result) const;

        void validateObject(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateArray(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateAllOf(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateAnyOf(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateOneOf(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateNot(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        void validateIfThenElse(
            const Document& document,
            const Document& schema,
            std::string_view path,
            ValidationResult& result,
            size_t currentDepth = 0) const;

        bool referenceExists(std::string_view reference) const noexcept;
        Document resolveReference(std::string_view reference) const;
        Document extractSubDocument(const Document& sourceDocument, std::string_view path) const;
        void buildAnchorIndex();
        void scanForAnchors(const Document& schema, const std::string& currentPath);
        void detectDraft();
        std::string draftToString() const noexcept;

        static constexpr size_t DEFAULT_MAX_DEPTH = 64;

        Document m_schema;
        bool m_schemaLoaded = false;
        bool m_strictMode = false;
        size_t m_maxDepth = DEFAULT_MAX_DEPTH; ///< Maximum validation depth (0 = unlimited)
        SchemaDraft m_schemaDraft = SchemaDraft::Unknown;
        mutable std::unordered_map<std::string, Document> m_refCache;
        std::unordered_map<std::string, std::string> m_anchorIndex;
    };
} // namespace dnv::vista::sdk::json
