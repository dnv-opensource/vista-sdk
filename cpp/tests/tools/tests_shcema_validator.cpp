#include <tools/json-validator/SchemaValidator.h>

#include <SchemasRegistry.h>

#include <doctest/doctest.h>

using namespace dnv::vista::sdk::json;

namespace
{
    Document parseOrFail(std::string_view json)
    {
        auto doc = Document::fromString(json);
        REQUIRE(doc.has_value());
        return *doc;
    }

    SchemaValidator makeValidator(std::string_view schemaJson)
    {
        SchemaValidator validator;
        REQUIRE(validator.load(schemaJson));
        return validator;
    }

    std::string_view embeddedSchemaText(std::string_view filename)
    {
        const auto* resource = dnv::vista::sdk::schemas::find(filename);
        REQUIRE(resource != nullptr);
        return resource->str();
    }
} // namespace

TEST_SUITE("json-validator::SchemaValidator")
{
    TEST_CASE("load - accepts a valid object schema, rejects malformed JSON")
    {
        SchemaValidator validator;
        CHECK(validator.load(std::string_view{ R"({"type": "object"})" }));
        CHECK(validator.hasSchema());

        SchemaValidator invalid;
        CHECK_FALSE(invalid.load(std::string_view{ "{ not valid json" }));
        CHECK_FALSE(invalid.hasSchema());
    }

    TEST_CASE("validate - throws if no schema loaded")
    {
        SchemaValidator validator;
        auto doc = parseOrFail(R"({"a": 1})");
        CHECK_THROWS_AS(validator.validate(doc), std::runtime_error);
    }

    TEST_CASE("type - matches and mismatches")
    {
        auto validator = makeValidator(R"({"type": "string"})");

        CHECK(validator.validate(parseOrFail(R"("hello")")).isValid());

        auto result = validator.validate(parseOrFail("42"));
        CHECK_FALSE(result.isValid());
        CHECK(result.errorCount() == 1);
        CHECK(result.error(0).constraint() == "type");
    }

    TEST_CASE("type - integer is accepted where number is expected, not vice versa")
    {
        auto numberValidator = makeValidator(R"({"type": "number"})");
        CHECK(numberValidator.validate(parseOrFail("42")).isValid());
        CHECK(numberValidator.validate(parseOrFail("3.14")).isValid());

        auto integerValidator = makeValidator(R"({"type": "integer"})");
        CHECK(integerValidator.validate(parseOrFail("42")).isValid());
        CHECK_FALSE(integerValidator.validate(parseOrFail("3.14")).isValid());
    }

    TEST_CASE("type - array of allowed types")
    {
        auto validator = makeValidator(R"({"type": ["string", "null"]})");
        CHECK(validator.validate(parseOrFail(R"("x")")).isValid());
        CHECK(validator.validate(parseOrFail("null")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("42")).isValid());
    }

    TEST_CASE("required and properties")
    {
        auto validator = makeValidator(R"({
            "type": "object",
            "required": ["name"],
            "properties": {
                "name": { "type": "string" },
                "age": { "type": "integer" }
            }
        })");

        CHECK(validator.validate(parseOrFail(R"({"name": "Alice"})")).isValid());

        auto missing = validator.validate(parseOrFail(R"({"age": 30})"));
        CHECK_FALSE(missing.isValid());
        CHECK(missing.error(0).constraint() == "required");

        auto wrongType = validator.validate(parseOrFail(R"({"name": "Alice", "age": "thirty"})"));
        CHECK_FALSE(wrongType.isValid());
    }

    TEST_CASE("additionalProperties - false rejects, schema validates, true/absent allows")
    {
        auto strict = makeValidator(R"({
            "type": "object",
            "properties": { "a": { "type": "string" } },
            "additionalProperties": false
        })");
        CHECK(strict.validate(parseOrFail(R"({"a": "x"})")).isValid());
        CHECK_FALSE(strict.validate(parseOrFail(R"({"a": "x", "b": 1})")).isValid());

        auto typed = makeValidator(R"({
            "type": "object",
            "properties": { "a": { "type": "string" } },
            "additionalProperties": { "type": "integer" }
        })");
        CHECK(typed.validate(parseOrFail(R"({"a": "x", "b": 1})")).isValid());
        CHECK_FALSE(typed.validate(parseOrFail(R"({"a": "x", "b": "not an int"})")).isValid());

        auto permissive = makeValidator(R"({"type": "object", "properties": {"a": {"type": "string"}}})");
        CHECK(permissive.validate(parseOrFail(R"({"a": "x", "b": 1})")).isValid());
    }

    TEST_CASE("numeric constraints")
    {
        auto validator = makeValidator(R"({
            "type": "number",
            "minimum": 0,
            "maximum": 100,
            "exclusiveMinimum": 0,
            "multipleOf": 5
        })");

        CHECK(validator.validate(parseOrFail("50")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("0")).isValid());   // violates exclusiveMinimum
        CHECK_FALSE(validator.validate(parseOrFail("150")).isValid()); // violates maximum
        CHECK_FALSE(validator.validate(parseOrFail("7")).isValid());   // violates multipleOf
    }

    TEST_CASE("string constraints - length and pattern")
    {
        auto validator = makeValidator(R"({
            "type": "string",
            "minLength": 2,
            "maxLength": 5,
            "pattern": "^[a-z]+$"
        })");

        CHECK(validator.validate(parseOrFail(R"("abc")")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail(R"("a")")).isValid());      // too short
        CHECK_FALSE(validator.validate(parseOrFail(R"("abcdef")")).isValid()); // too long
        CHECK_FALSE(validator.validate(parseOrFail(R"("ABC")")).isValid());    // pattern mismatch
    }

    TEST_CASE("string constraints - format date-time")
    {
        auto validator = makeValidator(R"({"type": "string", "format": "date-time"})");

        CHECK(validator.validate(parseOrFail(R"("2024-01-15T10:30:00Z")")).isValid());

        auto result = validator.validate(parseOrFail(R"("not-a-date")"));
        CHECK_FALSE(result.isValid());
        CHECK(result.error(0).constraint() == "format");
    }

    TEST_CASE("string constraints - format ignores unknown format names")
    {
        // Only "date-time" is implemented. Other format values are accepted without validation
        auto validator = makeValidator(R"({"type": "string", "format": "email"})");
        CHECK(validator.validate(parseOrFail(R"("not an email at all")")).isValid());
    }

    TEST_CASE("enum and const")
    {
        auto enumValidator = makeValidator(R"({"enum": ["a", "b", "c"]})");
        CHECK(enumValidator.validate(parseOrFail(R"("b")")).isValid());
        CHECK_FALSE(enumValidator.validate(parseOrFail(R"("z")")).isValid());

        auto constValidator = makeValidator(R"({"const": 42})");
        CHECK(constValidator.validate(parseOrFail("42")).isValid());
        CHECK_FALSE(constValidator.validate(parseOrFail("43")).isValid());
    }

    TEST_CASE("allOf - all subschemas must match")
    {
        auto validator = makeValidator(R"({
            "allOf": [
                { "type": "string" },
                { "minLength": 3 }
            ]
        })");

        CHECK(validator.validate(parseOrFail(R"("abcd")")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail(R"("ab")")).isValid());
    }

    TEST_CASE("anyOf - at least one subschema must match")
    {
        auto validator = makeValidator(R"({"anyOf": [{"type": "string"}, {"type": "integer"}]})");

        CHECK(validator.validate(parseOrFail(R"("x")")).isValid());
        CHECK(validator.validate(parseOrFail("1")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("true")).isValid());
    }

    TEST_CASE("oneOf - exactly one subschema must match")
    {
        auto validator = makeValidator(R"({
            "oneOf": [
                { "type": "number", "multipleOf": 3 },
                { "type": "number", "multipleOf": 5 }
            ]
        })");

        CHECK(validator.validate(parseOrFail("9")).isValid());  // only multiple of 3
        CHECK(validator.validate(parseOrFail("10")).isValid()); // only multiple of 5

        auto both = validator.validate(parseOrFail("15")); // multiple of both 3 and 5
        CHECK_FALSE(both.isValid());

        auto neither = validator.validate(parseOrFail("7"));
        CHECK_FALSE(neither.isValid());
    }

    TEST_CASE("not - value must not match the subschema")
    {
        auto validator = makeValidator(R"({"not": {"type": "string"}})");

        CHECK(validator.validate(parseOrFail("42")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail(R"("x")")).isValid());
    }

    TEST_CASE("if-then-else")
    {
        auto validator = makeValidator(R"({
            "if": { "type": "string" },
            "then": { "minLength": 3 },
            "else": { "type": "integer" }
        })");

        CHECK(validator.validate(parseOrFail(R"("abc")")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail(R"("ab")")).isValid());
        CHECK(validator.validate(parseOrFail("1")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("true")).isValid());
    }

    TEST_CASE("array constraints - length, uniqueItems, items")
    {
        auto validator = makeValidator(R"({
            "type": "array",
            "minItems": 1,
            "maxItems": 3,
            "uniqueItems": true,
            "items": { "type": "integer" }
        })");

        CHECK(validator.validate(parseOrFail("[1, 2, 3]")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("[]")).isValid());           // below minItems
        CHECK_FALSE(validator.validate(parseOrFail("[1, 2, 3, 4]")).isValid()); // above maxItems
        CHECK_FALSE(validator.validate(parseOrFail("[1, 1]")).isValid());       // duplicate
        CHECK_FALSE(validator.validate(parseOrFail(R"([1, "x"])")).isValid());  // wrong item type
    }

    TEST_CASE("$ref - resolves a local JSON pointer")
    {
        auto validator = makeValidator(R"({
            "definitions": {
                "positiveInt": { "type": "integer", "minimum": 0 }
            },
            "$ref": "#/definitions/positiveInt"
        })");

        CHECK(validator.validate(parseOrFail("5")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("-5")).isValid());
    }

    TEST_CASE("$ref - resolves via $anchor")
    {
        auto validator = makeValidator(R"({
            "definitions": {
                "named": { "$anchor": "PositiveInt", "type": "integer", "minimum": 0 }
            },
            "$ref": "#PositiveInt"
        })");

        CHECK(validator.validate(parseOrFail("5")).isValid());
        CHECK_FALSE(validator.validate(parseOrFail("-5")).isValid());
    }

    TEST_CASE("$ref - unresolved reference reports an error instead of throwing")
    {
        auto validator = makeValidator(R"({"$ref": "#/definitions/doesNotExist"})");

        ValidationResult result;
        CHECK_NOTHROW(result = validator.validate(parseOrFail("1")));
        CHECK_FALSE(result.isValid());
        CHECK(result.error(0).constraint() == "$ref");
    }

    TEST_CASE("$ref - circular reference is caught via maxDepth instead of infinite recursion")
    {
        auto validator = makeValidator(R"({
            "definitions": {
                "loop": { "$ref": "#/definitions/loop" }
            },
            "$ref": "#/definitions/loop"
        })");

        ValidationResult result;
        CHECK_NOTHROW(result = validator.validate(parseOrFail("1")));
        CHECK_FALSE(result.isValid());
    }

    TEST_CASE("metadata accessors - version, draft, title, description")
    {
        auto validator = makeValidator(R"({
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "title": "Example",
            "description": "An example schema",
            "type": "object"
        })");

        CHECK(validator.version() == "https://json-schema.org/draft/2020-12/schema");
        CHECK(validator.draft() == SchemaDraft::Draft202012);
        CHECK(validator.draftString() == "2020-12");
        CHECK(validator.title() == "Example");
        CHECK(validator.description() == "An example schema");
    }

    TEST_CASE("boolean schemas - true accepts everything, false rejects everything")
    {
        auto alwaysValid = makeValidator("true");
        CHECK(alwaysValid.validate(parseOrFail(R"({"anything": 1})")).isValid());

        auto alwaysInvalid = makeValidator("false");
        CHECK_FALSE(alwaysInvalid.validate(parseOrFail("1")).isValid());
    }

    TEST_CASE("clear - resets loaded schema state")
    {
        auto validator = makeValidator(R"({"type": "string"})");
        REQUIRE(validator.hasSchema());

        validator.clear();
        CHECK_FALSE(validator.hasSchema());
        CHECK_THROWS_AS(validator.validate(parseOrFail(R"("x")")), std::runtime_error);
    }

    TEST_CASE("ValidationError::toString - includes path, message and constraint details")
    {
        ValidationError error{ "/name", "Type mismatch", "type", "string", "integer" };
        auto str = error.toString();
        CHECK(str.find("/name") != std::string::npos);
        CHECK(str.find("Type mismatch") != std::string::npos);
        CHECK(str.find("string") != std::string::npos);
        CHECK(str.find("integer") != std::string::npos);
    }

    TEST_CASE("real DataChannelList schema - embedded sample validates successfully")
    {
        auto validator = makeValidator(embeddedSchemaText("DataChannelList.schema.json"));
        auto sample = parseOrFail(embeddedSchemaText("DataChannelList.sample.json"));

        auto result = validator.validate(sample);
        INFO(result.errorSummary());
        CHECK(result.isValid());
    }

    TEST_CASE("real DataChannelList schema - compact sample also validates successfully")
    {
        auto validator = makeValidator(embeddedSchemaText("DataChannelList.schema.json"));
        auto sample = parseOrFail(embeddedSchemaText("DataChannelList.sample.compact.json"));

        auto result = validator.validate(sample);
        INFO(result.errorSummary());
        CHECK(result.isValid());
    }

    TEST_CASE("real DataChannelList schema - rejects a sample with a malformed date-time")
    {
        auto validator = makeValidator(embeddedSchemaText("DataChannelList.schema.json"));
        auto sample = parseOrFail(embeddedSchemaText("DataChannelList.sample.json"));

        // Corrupt the header's TimeStamp so it no longer conforms to format: date-time
        REQUIRE(sample.contains("Package"));
        Document& package = sample.at("Package");
        REQUIRE(package.contains("Header"));
        Document& header = package.at("Header");
        REQUIRE(header.contains("DateCreated"));
        header.set("DateCreated", Document{ std::string{ "not-a-date" } });

        auto result = validator.validate(sample);
        CHECK_FALSE(result.isValid());
    }

    TEST_CASE("real TimeSeriesData schema - embedded sample validates successfully")
    {
        auto validator = makeValidator(embeddedSchemaText("TimeSeriesData.schema.json"));
        auto sample = parseOrFail(embeddedSchemaText("TimeSeriesData.sample.json"));

        auto result = validator.validate(sample);
        INFO(result.errorSummary());
        CHECK(result.isValid());
    }
}
