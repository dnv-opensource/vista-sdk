#include <doctest/doctest.h>

#include <JSON/Json.h>

#include <cstdint>
#include <string>
#include <vector>

namespace dnv::vista::sdk::tests
{
    using namespace dnv::vista::sdk::json;

    TEST_SUITE("Json::Parser")
    {
        TEST_CASE("null")
        {
            auto doc = Document::fromString("null");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Null);
        }

        TEST_CASE("boolean true")
        {
            auto doc = Document::fromString("true");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Boolean);
            CHECK_EQ(doc->root<bool>(), true);
        }

        TEST_CASE("boolean false")
        {
            auto doc = Document::fromString("false");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->root<bool>(), false);
        }

        TEST_CASE("integer")
        {
            auto doc = Document::fromString("42");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Integer);
            CHECK_EQ(doc->root<int64_t>(), int64_t{ 42 });
        }

        TEST_CASE("negative integer")
        {
            auto doc = Document::fromString("-7");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->root<int64_t>(), int64_t{ -7 });
        }

        TEST_CASE("double")
        {
            auto doc = Document::fromString("3.14");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Double);
            CHECK(doc->root<double>().value() == doctest::Approx(3.14));
        }

        TEST_CASE("scientific notation")
        {
            auto doc = Document::fromString("1.5e2");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Double);
            CHECK(doc->root<double>().value() == doctest::Approx(150.0));
        }

        TEST_CASE("string")
        {
            auto doc = Document::fromString(R"("hello")");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::String);
            CHECK_EQ(doc->root<std::string>(), "hello");
        }

        TEST_CASE("string with escape sequences")
        {
            auto doc = Document::fromString(R"("line1\nline2\ttabbed")");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->root<std::string>(), "line1\nline2\ttabbed");
        }

        TEST_CASE("string with unicode escape")
        {
            auto doc = Document::fromString(R"("A")");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->root<std::string>(), "A");
        }

        TEST_CASE("string with surrogate pair")
        {
            // U+1F600 GRINNING FACE: encoded as 😀
            auto doc = Document::fromString(R"("😀")");
            REQUIRE(doc.has_value());
            auto str = doc->root<std::string>();
            REQUIRE(str.has_value());
            // UTF-8 encoding of U+1F600: 0xF0 0x9F 0x98 0x80
            CHECK_EQ(str->size(), 4u);
            CHECK_EQ(static_cast<unsigned char>((*str)[0]), 0xF0u);
            CHECK_EQ(static_cast<unsigned char>((*str)[1]), 0x9Fu);
            CHECK_EQ(static_cast<unsigned char>((*str)[2]), 0x98u);
            CHECK_EQ(static_cast<unsigned char>((*str)[3]), 0x80u);
        }

        TEST_CASE("truncated high surrogate returns nullopt")
        {
            // High surrogate with no following \uXXXX
            CHECK_FALSE(Document::fromString("\"\\uD800\"").has_value());
            CHECK_FALSE(Document::fromString("\"\\uD800\\u\"").has_value());
            CHECK_FALSE(Document::fromString("\"\\uD800\\uDC\"").has_value());
        }

        TEST_CASE("isolated low surrogate returns nullopt")
        {
            CHECK_FALSE(Document::fromString("\"\\uDC00\"").has_value());
        }

        TEST_CASE("empty array")
        {
            auto doc = Document::fromString("[]");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Array);
            CHECK_EQ(doc->size(), 0u);
        }

        TEST_CASE("array of integers")
        {
            auto doc = Document::fromString("[1, 2, 3]");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->size(), 3u);
            CHECK_EQ(doc->at(0).root<int64_t>(), int64_t{ 1 });
            CHECK_EQ(doc->at(1).root<int64_t>(), int64_t{ 2 });
            CHECK_EQ(doc->at(2).root<int64_t>(), int64_t{ 3 });
        }

        TEST_CASE("empty object")
        {
            auto doc = Document::fromString("{}");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Object);
            CHECK_EQ(doc->size(), 0u);
        }

        TEST_CASE("simple object")
        {
            auto doc = Document::fromString(R"({"name":"Alice","age":30})");
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Object);
            CHECK_EQ((*doc)["name"].root<std::string>(), "Alice");
            CHECK_EQ((*doc)["age"].root<int64_t>(), int64_t{ 30 });
        }

        TEST_CASE("nested object")
        {
            auto doc = Document::fromString(R"({"a":{"b":{"c":99}}})");
            REQUIRE(doc.has_value());
            CHECK_EQ((*doc)["a"]["b"]["c"].root<int64_t>(), int64_t{ 99 });
        }

        TEST_CASE("UTF-8 BOM is skipped")
        {
            std::string json = "\xEF\xBB\xBF{\"k\":1}";
            auto doc = Document::fromString(json);
            REQUIRE(doc.has_value());
            CHECK_EQ(doc->type(), Type::Object);
        }

        TEST_CASE("whitespace only returns nullopt")
        {
            CHECK_FALSE(Document::fromString("   ").has_value());
        }

        TEST_CASE("empty string returns nullopt")
        {
            CHECK_FALSE(Document::fromString("").has_value());
        }

        TEST_CASE("invalid json returns nullopt")
        {
            CHECK_FALSE(Document::fromString("{bad}").has_value());
        }
    }

    TEST_SUITE("Json::Document")
    {
        TEST_CASE("default construction is object")
        {
            Document doc;
            CHECK_EQ(doc.type(), Type::Object);
        }

        TEST_CASE("null construction")
        {
            Document doc{ nullptr };
            CHECK_EQ(doc.type(), Type::Null);
        }

        TEST_CASE("static factory object()")
        {
            auto doc = Document::object();
            CHECK_EQ(doc.type(), Type::Object);
            CHECK(doc.isEmpty());
        }

        TEST_CASE("static factory array()")
        {
            auto doc = Document::array();
            CHECK_EQ(doc.type(), Type::Array);
            CHECK(doc.isEmpty());
        }

        TEST_CASE("set and find")
        {
            Document doc = Document::object();
            doc.set("x", Document{ int64_t{ 42 } });
            auto* found = doc.find("x");
            REQUIRE(found != nullptr);
            CHECK_EQ(found->root<int64_t>(), int64_t{ 42 });
        }

        TEST_CASE("contains")
        {
            Document doc = Document::object();
            doc.set("key", Document{ std::string{ "val" } });
            CHECK(doc.contains("key"));
            CHECK_FALSE(doc.contains("missing"));
        }

        TEST_CASE("contains - empty key is not special-cased")
        {
            Document obj = Document::object();
            CHECK_FALSE(obj.contains(""));

            Document withEmptyKey = Document::object();
            withEmptyKey.set("", Document{ std::string{ "val" } });
            CHECK(withEmptyKey.contains(""));
        }

        TEST_CASE("contains - non-object types never contain a key")
        {
            CHECK_FALSE(Document{ 42 }.contains("key"));
            CHECK_FALSE(Document{ 42 }.contains(""));
            CHECK_FALSE(Document::array().contains("key"));
        }

        TEST_CASE("operator[] creates key on write")
        {
            Document doc = Document::object();
            doc["new_key"] = std::string{ "created" };
            CHECK(doc.contains("new_key"));
        }

        TEST_CASE("operator[] const returns null for missing key")
        {
            const Document doc = Document::object();
            CHECK_EQ(doc["missing"].type(), Type::Object);
        }

        TEST_CASE("at throws on missing key")
        {
            Document doc = Document::object();
            CHECK_THROWS_AS(doc.at("nope"), std::out_of_range);
        }

        TEST_CASE("push_back and size")
        {
            Document doc = Document::array();
            doc.push_back(Document{ int64_t{ 1 } });
            doc.push_back(Document{ int64_t{ 2 } });
            CHECK_EQ(doc.size(), 2u);
        }

        TEST_CASE("front and back")
        {
            Document doc = Document::array();
            doc.push_back(Document{ int64_t{ 1 } });
            doc.push_back(Document{ int64_t{ 2 } });
            doc.push_back(Document{ int64_t{ 3 } });

            CHECK_EQ(doc.front().root<int64_t>(), int64_t{ 1 });
            CHECK_EQ(doc.back().root<int64_t>(), int64_t{ 3 });

            const Document& constDoc = doc;
            CHECK_EQ(constDoc.front().root<int64_t>(), int64_t{ 1 });
            CHECK_EQ(constDoc.back().root<int64_t>(), int64_t{ 3 });
        }

        TEST_CASE("front and back throw on empty array")
        {
            Document doc = Document::array();
            CHECK_THROWS_AS((void)doc.front(), std::out_of_range);
            CHECK_THROWS_AS((void)doc.back(), std::out_of_range);

            const Document& constDoc = doc;
            CHECK_THROWS_AS((void)constDoc.front(), std::out_of_range);
            CHECK_THROWS_AS((void)constDoc.back(), std::out_of_range);
        }

        TEST_CASE("erase key")
        {
            Document doc = Document::object();
            doc.set("a", Document{ int64_t{ 1 } });
            doc.set("b", Document{ int64_t{ 2 } });
            CHECK_EQ(doc.erase("a"), 1u);
            CHECK_FALSE(doc.contains("a"));
            CHECK(doc.contains("b"));
        }

        TEST_CASE("clear")
        {
            Document doc = Document::array();
            doc.push_back(Document{ int64_t{ 1 } });
            doc.clear();
            CHECK(doc.isEmpty());
        }

        TEST_CASE("objectBegin / objectEnd iteration")
        {
            auto doc = Document::fromString(R"({"a":1,"b":2})");
            REQUIRE(doc.has_value());
            int count = 0;
            for (auto it = doc->objectBegin(); it != doc->objectEnd(); ++it)
            {
                ++count;
            }
            CHECK_EQ(count, 2);
        }

        TEST_CASE("rootRef for Array avoids copy")
        {
            Document doc = Document::array();
            doc.push_back(Document{ int64_t{ 7 } });
            auto ref = doc.rootRef<Array>();
            REQUIRE(ref.has_value());
            CHECK_EQ(ref->get().size(), 1u);
        }

        TEST_CASE("visit dispatches on type")
        {
            Document doc{ int64_t{ 99 } };
            bool visited = false;
            doc.visit([&](auto&& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, int64_t>)
                {
                    visited = true;
                    CHECK_EQ(val, int64_t{ 99 });
                }
            });
            CHECK(visited);
        }

        TEST_CASE("operator== equal documents")
        {
            auto a = Document::fromString(R"({"k":1})");
            auto b = Document::fromString(R"({"k":1})");
            REQUIRE(a.has_value());
            REQUIRE(b.has_value());
            CHECK(*a == *b);
        }

        TEST_CASE("operator== different documents")
        {
            auto a = Document::fromString(R"({"k":1})");
            auto b = Document::fromString(R"({"k":2})");
            REQUIRE(a.has_value());
            REQUIRE(b.has_value());
            CHECK_FALSE(*a == *b);
        }
    }

    TEST_SUITE("Json::Builder")
    {
        TEST_CASE("compact object")
        {
            Builder b;
            b.writeStartObject().write("name", "Alice").write("age", int64_t{ 30 }).writeEndObject();
            CHECK_EQ(b.toString(), R"({"name":"Alice","age":30})");
        }

        TEST_CASE("compact array")
        {
            Builder b;
            b.writeStartArray().write(int64_t{ 1 }).write(int64_t{ 2 }).write(int64_t{ 3 }).writeEndArray();
            CHECK_EQ(b.toString(), "[1,2,3]");
        }

        TEST_CASE("null value")
        {
            Builder b;
            b.writeStartObject().write("x", nullptr).writeEndObject();
            CHECK_EQ(b.toString(), R"({"x":null})");
        }

        TEST_CASE("boolean values")
        {
            Builder b;
            b.writeStartObject().write("t", true).write("f", false).writeEndObject();
            CHECK_EQ(b.toString(), R"({"t":true,"f":false})");
        }

        TEST_CASE("double value")
        {
            Builder b;
            b.writeStartObject().write("pi", 3.14).writeEndObject();
            auto result = b.toString();
            auto doc = Document::fromString(result);
            REQUIRE(doc.has_value());
            CHECK((*doc)["pi"].root<double>().value() == doctest::Approx(3.14));
        }

        TEST_CASE("nested object via writeKey")
        {
            Builder b;
            b.writeStartObject()
                .writeKey("inner")
                .writeStartObject()
                .write("v", int64_t{ 1 })
                .writeEndObject()
                .writeEndObject();
            CHECK_EQ(b.toString(), R"({"inner":{"v":1}})");
        }

        TEST_CASE("writeArray from container")
        {
            std::vector<int64_t> vals{ 10, 20, 30 };
            Builder b;
            b.writeStartObject().writeArray("items", vals).writeEndObject();
            CHECK_EQ(b.toString(), R"({"items":[10,20,30]})");
        }

        TEST_CASE("string escaping")
        {
            Builder b;
            b.writeStartObject().write("msg", "line1\nline2\ttab\"quote").writeEndObject();
            auto result = b.toString();
            auto doc = Document::fromString(result);
            REQUIRE(doc.has_value());
            CHECK_EQ((*doc)["msg"].root<std::string>(), "line1\nline2\ttab\"quote");
        }

        TEST_CASE("write Document value")
        {
            auto inner = Document::fromString(R"({"a":1})");
            REQUIRE(inner.has_value());
            Builder b;
            b.writeStartObject().write("data", *inner).writeEndObject();
            auto result = b.toString();
            auto doc = Document::fromString(result);
            REQUIRE(doc.has_value());
            CHECK_EQ((*doc)["data"]["a"].root<int64_t>(), int64_t{ 1 });
        }

        TEST_CASE("isValid after close")
        {
            Builder b;
            b.writeStartObject().writeEndObject();
            CHECK(b.isValid());
        }

        TEST_CASE("reset allows reuse")
        {
            Builder b;
            b.writeStartObject().write("x", int64_t{ 1 }).writeEndObject();
            b.reset();
            b.writeStartArray().write(int64_t{ 2 }).writeEndArray();
            CHECK_EQ(b.toString(), "[2]");
        }

        TEST_CASE("round-trip fromString -> toString -> fromString")
        {
            const std::string original = R"({"key":"value","nums":[1,2,3],"flag":true,"nothing":null})";
            auto doc = Document::fromString(original);
            REQUIRE(doc.has_value());
            auto serialized = doc->toString();
            auto doc2 = Document::fromString(serialized);
            REQUIRE(doc2.has_value());
            CHECK(*doc == *doc2);
        }
    }
} // namespace dnv::vista::sdk::tests
