#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

namespace test_adapters
{
    using dnv::vista::sdk::transport::serialization::json::SerializableDocument;
    using dnv::vista::sdk::transport::serialization::json::ToSerializableDocumentTag;
    using dnv::vista::sdk::transport::serialization::json::FromSerializableDocumentTag;

    struct SimpleJson
    {
        std::string text;
        int number{ 0 };
    };

    inline SerializableDocument tag_invoke(ToSerializableDocumentTag, const SimpleJson& v)
    {
        auto doc = SerializableDocument::object();
        doc.set("text", SerializableDocument{ v.text });
        doc.set("number", SerializableDocument{ static_cast<std::int64_t>(v.number) });
        return doc;
    }

    inline void tag_invoke(FromSerializableDocumentTag, const SerializableDocument& doc, SimpleJson& out)
    {
        if (const auto* t = doc.find("text"))
        {
            out.text = t->asString();
        }
        if (const auto* n = doc.find("number"))
        {
            out.number = static_cast<int>(n->asInteger());
        }
    }
} // namespace test_adapters

namespace dnv::vista::sdk::tests
{
    using dnv::vista::sdk::transport::serialization::json::SerializableDocument;

    TEST_SUITE("SerializableDocument")
    {
        TEST_CASE("null - default construction")
        {
            SerializableDocument doc;
            CHECK(doc.isNull());
            CHECK_EQ(doc.kind(), SerializableDocument::Kind::Null);
        }

        TEST_CASE("boolean - construction")
        {
            SerializableDocument t{ true };
            SerializableDocument f{ false };

            CHECK(t.isBoolean());
            CHECK_EQ(t.kind(), SerializableDocument::Kind::Boolean);
            CHECK(t.asBoolean());
            CHECK_FALSE(f.asBoolean());
        }

        TEST_CASE("integer - construction")
        {
            SerializableDocument doc{ std::int64_t{ 42 } };
            CHECK(doc.isInteger());
            CHECK_EQ(doc.kind(), SerializableDocument::Kind::Integer);
            CHECK_EQ(doc.asInteger(), 42);
        }

        TEST_CASE("integer - negative value")
        {
            SerializableDocument doc{ std::int64_t{ -1000 } };
            CHECK(doc.isInteger());
            CHECK_EQ(doc.asInteger(), -1000);
        }

        TEST_CASE("double - construction")
        {
            SerializableDocument doc{ 3.14 };
            CHECK(doc.isDouble());
            CHECK_EQ(doc.kind(), SerializableDocument::Kind::Double);
            CHECK_EQ(doc.asDouble(), doctest::Approx(3.14));
        }

        TEST_CASE("string - from std::string")
        {
            SerializableDocument doc{ std::string{ "hello" } };
            CHECK(doc.isString());
            CHECK_EQ(doc.kind(), SerializableDocument::Kind::String);
            CHECK_EQ(doc.asString(), "hello");
        }

        TEST_CASE("string - from string_view")
        {
            SerializableDocument doc{ std::string_view{ "world" } };
            CHECK(doc.isString());
            CHECK_EQ(doc.asString(), "world");
        }

        TEST_CASE("array - construction from Array")
        {
            SerializableDocument::Array arr;
            arr.emplace_back(std::int64_t{ 1 });
            arr.emplace_back(std::int64_t{ 2 });
            arr.emplace_back(std::int64_t{ 3 });

            SerializableDocument doc{ std::move(arr) };
            CHECK(doc.isArray());
            CHECK_EQ(doc.kind(), SerializableDocument::Kind::Array);
            CHECK_EQ(doc.asArray().size(), 3u);
        }

        TEST_CASE("array - static factory")
        {
            auto doc = SerializableDocument::array();
            CHECK(doc.isArray());
            CHECK(doc.asArray().empty());
        }

        TEST_CASE("object - static factory")
        {
            auto doc = SerializableDocument::object();
            CHECK(doc.isObject());
            CHECK_EQ(doc.kind(), SerializableDocument::Kind::Object);
            CHECK(doc.asObject().empty());
        }

        TEST_CASE("kind - only one kind is true")
        {
            SerializableDocument null;
            CHECK(null.isNull());
            CHECK_FALSE(null.isBoolean());
            CHECK_FALSE(null.isInteger());
            CHECK_FALSE(null.isDouble());
            CHECK_FALSE(null.isString());
            CHECK_FALSE(null.isArray());
            CHECK_FALSE(null.isObject());

            SerializableDocument str{ std::string{ "test" } };
            CHECK_FALSE(str.isNull());
            CHECK_FALSE(str.isBoolean());
            CHECK(str.isString());
        }

        TEST_CASE("asBoolean - throws if not boolean")
        {
            SerializableDocument doc{ std::int64_t{ 1 } };
            CHECK_THROWS_AS((void)doc.asBoolean(), std::bad_variant_access);
        }

        TEST_CASE("asInteger - throws if not integer")
        {
            SerializableDocument doc{ 1.0 };
            CHECK_THROWS_AS((void)doc.asInteger(), std::bad_variant_access);
        }

        TEST_CASE("asDouble - throws if not double")
        {
            SerializableDocument doc{ std::string{ "3.14" } };
            CHECK_THROWS_AS((void)doc.asDouble(), std::bad_variant_access);
        }

        TEST_CASE("asString - throws if not string")
        {
            SerializableDocument doc{ true };
            CHECK_THROWS_AS((void)doc.asString(), std::bad_variant_access);
        }

        TEST_CASE("asArray - throws if not array")
        {
            SerializableDocument doc;
            CHECK_THROWS_AS((void)doc.asArray(), std::bad_variant_access);
        }

        TEST_CASE("asObject - throws if not object")
        {
            SerializableDocument doc{ std::int64_t{ 0 } };
            CHECK_THROWS_AS((void)doc.asObject(), std::bad_variant_access);
        }

        TEST_CASE("object - set and find")
        {
            auto doc = SerializableDocument::object();
            doc.set("key", SerializableDocument{ std::string{ "value" } });

            const auto* found = doc.find("key");
            REQUIRE(found != nullptr);
            CHECK(found->isString());
            CHECK_EQ(found->asString(), "value");
        }

        TEST_CASE("object - contains")
        {
            auto doc = SerializableDocument::object();
            doc.set("x", SerializableDocument{ std::int64_t{ 1 } });

            CHECK(doc.contains("x"));
            CHECK_FALSE(doc.contains("y"));
        }

        TEST_CASE("object - find returns nullptr on missing key")
        {
            auto doc = SerializableDocument::object();
            CHECK_EQ(doc.find("missing"), nullptr);
        }

        TEST_CASE("object - find returns nullptr on non-object")
        {
            SerializableDocument doc{ std::string{ "not an object" } };
            CHECK_EQ(doc.find("key"), nullptr);
        }

        TEST_CASE("object - set replaces existing key in place")
        {
            auto doc = SerializableDocument::object();
            doc.set("k", SerializableDocument{ std::int64_t{ 1 } });
            doc.set("k", SerializableDocument{ std::int64_t{ 2 } });

            CHECK_EQ(doc.asObject().size(), 1u);
            REQUIRE(doc.find("k") != nullptr);
            CHECK_EQ(doc.find("k")->asInteger(), 2);
        }

        TEST_CASE("object - set preserves insertion order")
        {
            auto doc = SerializableDocument::object();
            doc.set("a", SerializableDocument{ std::int64_t{ 1 } });
            doc.set("b", SerializableDocument{ std::int64_t{ 2 } });
            doc.set("c", SerializableDocument{ std::int64_t{ 3 } });

            const auto& obj = doc.asObject();
            REQUIRE(obj.size() == 3u);
            CHECK_EQ(obj[0].first, "a");
            CHECK_EQ(obj[1].first, "b");
            CHECK_EQ(obj[2].first, "c");
        }

        TEST_CASE("object - set on non-object resets to object")
        {
            SerializableDocument doc{ std::string{ "was a string" } };
            doc.set("key", SerializableDocument{ true });

            CHECK(doc.isObject());
            CHECK(doc.contains("key"));
        }

        TEST_CASE("array - push_back appends")
        {
            auto doc = SerializableDocument::array();
            doc.push_back(SerializableDocument{ std::int64_t{ 10 } });
            doc.push_back(SerializableDocument{ std::int64_t{ 20 } });

            const auto& arr = doc.asArray();
            REQUIRE(arr.size() == 2u);
            CHECK_EQ(arr[0].asInteger(), 10);
            CHECK_EQ(arr[1].asInteger(), 20);
        }

        TEST_CASE("array - push_back on non-array resets to array")
        {
            SerializableDocument doc{ std::string{ "was a string" } };
            doc.push_back(SerializableDocument{ true });

            CHECK(doc.isArray());
            CHECK_EQ(doc.asArray().size(), 1u);
        }

        TEST_CASE("object - findMutable returns nullptr on missing key")
        {
            auto doc = SerializableDocument::object();
            CHECK_EQ(doc.find("missing"), nullptr);
        }

        TEST_CASE("object - findMutable returns nullptr on non-object")
        {
            SerializableDocument doc{ std::string{ "not an object" } };
            CHECK_EQ(doc.find("key"), nullptr);
        }

        TEST_CASE("object - findMutable write mutates the parent document")
        {
            auto doc = SerializableDocument::object();
            doc.set("key", SerializableDocument{ std::string{ "before" } });

            auto* found = doc.find("key");
            REQUIRE(found != nullptr);
            CHECK(found->isString());
            REQUIRE_EQ(found->asString(), "before");

            *found = SerializableDocument{ std::string{ "after" } };

            REQUIRE(doc.find("key") != nullptr);
            CHECK_EQ(doc.find("key")->asString(), "after");
        }

        TEST_CASE("object - findMutable nested write mutates the parent document")
        {
            auto doc = SerializableDocument::object();
            doc.set("nested", SerializableDocument::object());

            auto* nested = doc.find("nested");
            REQUIRE(nested != nullptr);
            REQUIRE(nested->isObject());

            nested->set("inner", SerializableDocument{ std::int64_t{ 7 } });

            REQUIRE(doc.find("nested") != nullptr);
            REQUIRE(doc.find("nested")->isObject());
            const auto* inner = doc.find("nested")->find("inner");
            REQUIRE(inner != nullptr);
            CHECK_EQ(inner->asInteger(), 7);
        }

        TEST_CASE("object - non-const asObject allows in-place mutation")
        {
            auto doc = SerializableDocument::object();
            doc.set("x", SerializableDocument{ std::int64_t{ 1 } });

            auto& obj = doc.asObject();
            REQUIRE_EQ(obj.size(), 1u);
            obj[0].second = SerializableDocument{ std::int64_t{ 99 } };

            REQUIRE(doc.find("x") != nullptr);
            CHECK_EQ(doc.find("x")->asInteger(), 99);
        }

        TEST_CASE("array - non-const asArray allows in-place mutation")
        {
            auto doc = SerializableDocument::array();
            doc.push_back(SerializableDocument{ std::int64_t{ 1 } });
            doc.push_back(SerializableDocument{ std::int64_t{ 2 } });

            auto& arr = doc.asArray();
            REQUIRE_EQ(arr.size(), 2u);
            arr[1] = SerializableDocument{ std::int64_t{ 42 } };

            const auto& readBack = doc.asArray();
            REQUIRE_EQ(readBack.size(), 2u);
            const auto value = readBack[1].asInteger();
            CHECK_EQ(value, 42);
        }

        TEST_CASE("object - findMutable then push_back on a nested array")
        {
            auto doc = SerializableDocument::object();
            doc.set("list", SerializableDocument::array());

            auto* list = doc.find("list");
            REQUIRE(list != nullptr);
            REQUIRE(list->isArray());

            list->push_back(SerializableDocument{ std::int64_t{ 1 } });
            list->push_back(SerializableDocument{ std::int64_t{ 2 } });

            REQUIRE(doc.find("list") != nullptr);
            CHECK_EQ(doc.find("list")->asArray().size(), 2u);
        }

        TEST_CASE("equality - same primitive values")
        {
            CHECK_EQ(SerializableDocument{}, SerializableDocument{});
            CHECK_EQ(SerializableDocument{ true }, SerializableDocument{ true });
            CHECK_EQ(SerializableDocument{ std::int64_t{ 42 } }, SerializableDocument{ std::int64_t{ 42 } });
            CHECK_EQ(SerializableDocument{ 1.5 }, SerializableDocument{ 1.5 });
            CHECK_EQ(SerializableDocument{ std::string{ "hi" } }, SerializableDocument{ std::string{ "hi" } });
        }

        TEST_CASE("equality - different types are not equal")
        {
            CHECK_NE(SerializableDocument{}, SerializableDocument{ false });
            CHECK_NE(SerializableDocument{ std::int64_t{ 1 } }, SerializableDocument{ 1.0 });
            CHECK_NE(SerializableDocument{ std::string{ "true" } }, SerializableDocument{ true });
        }

        TEST_CASE("equality - objects with same content")
        {
            auto a = SerializableDocument::object();
            a.set("x", SerializableDocument{ std::int64_t{ 1 } });

            auto b = SerializableDocument::object();
            b.set("x", SerializableDocument{ std::int64_t{ 1 } });

            CHECK_EQ(a, b);
        }

        TEST_CASE("equality - objects with different insertion order are not equal")
        {
            auto a = SerializableDocument::object();
            a.set("x", SerializableDocument{ std::int64_t{ 1 } });
            a.set("y", SerializableDocument{ std::int64_t{ 2 } });

            auto b = SerializableDocument::object();
            b.set("y", SerializableDocument{ std::int64_t{ 2 } });
            b.set("x", SerializableDocument{ std::int64_t{ 1 } });

            CHECK_NE(a, b);
        }

        TEST_CASE("copy - produces independent copy")
        {
            auto original = SerializableDocument::object();
            original.set("k", SerializableDocument{ std::int64_t{ 99 } });

            auto copy = original;
            copy.set("k", SerializableDocument{ std::int64_t{ 0 } });

            REQUIRE(original.find("k") != nullptr);
            CHECK_EQ(original.find("k")->asInteger(), 99);
        }

        TEST_CASE("move - leaves source in valid state")
        {
            auto doc = SerializableDocument::object();
            doc.set("k", SerializableDocument{ std::string{ "v" } });

            auto moved = std::move(doc);
            CHECK(moved.isObject());
            CHECK(moved.contains("k"));
        }

        TEST_CASE("CPO - concept constraints")
        {
            CHECK_FALSE(dnv::vista::sdk::transport::serialization::json::SerializableToDocument<int>);
            CHECK_FALSE(dnv::vista::sdk::transport::serialization::json::SerializableFromDocument<int>);
            CHECK(dnv::vista::sdk::transport::serialization::json::SerializableToDocument<test_adapters::SimpleJson>);
            CHECK(dnv::vista::sdk::transport::serialization::json::SerializableFromDocument<test_adapters::SimpleJson>);
        }

        TEST_CASE("CPO - toSerializableDocument via tag_invoke")
        {
            using dnv::vista::sdk::transport::serialization::json::toSerializableDocument;

            test_adapters::SimpleJson input{ "hello", 42 };
            auto doc = toSerializableDocument(input);

            CHECK(doc.isObject());
            REQUIRE(doc.find("text") != nullptr);
            CHECK_EQ(doc.find("text")->asString(), "hello");
            REQUIRE(doc.find("number") != nullptr);
            CHECK_EQ(doc.find("number")->asInteger(), 42);
        }

        TEST_CASE("CPO - fromSerializableDocument via tag_invoke")
        {
            using dnv::vista::sdk::transport::serialization::json::fromSerializableDocument;

            auto doc = SerializableDocument::object();
            doc.set("text", SerializableDocument{ std::string{ "world" } });
            doc.set("number", SerializableDocument{ std::int64_t{ 7 } });

            test_adapters::SimpleJson out;
            fromSerializableDocument(doc, out);

            CHECK_EQ(out.text, "world");
            CHECK_EQ(out.number, 7);
        }

        TEST_CASE("CPO - roundtrip via from<T> and to<T>")
        {
            test_adapters::SimpleJson original{ "roundtrip", 99 };

            auto doc = SerializableDocument::from(original);
            auto restored = doc.to<test_adapters::SimpleJson>();

            CHECK_EQ(restored.text, "roundtrip");
            CHECK_EQ(restored.number, 99);
        }
    }
} // namespace dnv::vista::sdk::tests
