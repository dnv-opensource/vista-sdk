#include <doctest/doctest.h>

#include <dnv/VistaSDK.h>

#include <sstream>
#include <string>
#include <utility>

namespace dnv::vista::sdk::tests
{
    TEST_SUITE("StringBuilder")
    {
        TEST_CASE("default construction is empty")
        {
            StringBuilder sb;
            CHECK(sb.isEmpty());
            CHECK_EQ(sb.size(), 0u);
            CHECK_GE(sb.capacity(), 256u);
        }

        TEST_CASE("construction with initial capacity")
        {
            StringBuilder sb{ 1024 };
            CHECK(sb.isEmpty());
            CHECK_GE(sb.capacity(), 1024u);
        }

        TEST_CASE("append string_view stays on stack buffer for small content")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "hello" }).append(std::string_view{ ", " }).append(std::string_view{ "world" });

            CHECK_EQ(sb.toString(), "hello, world");
            CHECK_EQ(sb.size(), 12u);
            CHECK_LE(sb.capacity(), 256u);
        }

        TEST_CASE("append char")
        {
            StringBuilder sb;
            sb.append('a').append('b').append('c');
            CHECK_EQ(sb.toString(), "abc");
        }

        TEST_CASE("append empty string_view is a no-op")
        {
            StringBuilder sb;
            sb.append(std::string_view{});
            CHECK(sb.isEmpty());
        }

        TEST_CASE("append transitions from stack to heap past 256 bytes")
        {
            StringBuilder sb;
            const std::string chunk(100, 'x');
            sb.append(chunk).append(chunk).append(chunk);

            CHECK_EQ(sb.size(), 300u);
            CHECK_GT(sb.capacity(), 256u);
            CHECK_EQ(sb.toString(), chunk + chunk + chunk);
        }

        TEST_CASE("append signed integer")
        {
            StringBuilder sb;
            sb.append(std::int64_t{ 42 });
            CHECK_EQ(sb.toString(), "42");
        }

        TEST_CASE("append signed integer negative and extremes")
        {
            {
                StringBuilder sb;
                sb.append(std::int64_t{ -12345 });
                CHECK_EQ(sb.toString(), "-12345");
            }
            {
                StringBuilder sb;
                sb.append(std::numeric_limits<std::int64_t>::min());
                CHECK_EQ(sb.toString(), std::to_string(std::numeric_limits<std::int64_t>::min()));
            }
            {
                StringBuilder sb;
                sb.append(std::numeric_limits<std::int64_t>::max());
                CHECK_EQ(sb.toString(), std::to_string(std::numeric_limits<std::int64_t>::max()));
            }
        }

        TEST_CASE("append unsigned integer")
        {
            StringBuilder sb;
            sb.append(std::uint64_t{ 42 });
            CHECK_EQ(sb.toString(), "42");

            StringBuilder sbMax;
            sbMax.append(std::numeric_limits<std::uint64_t>::max());
            CHECK_EQ(sbMax.toString(), std::to_string(std::numeric_limits<std::uint64_t>::max()));
        }

        TEST_CASE("append double")
        {
            StringBuilder sb;
            sb.append(3.14);
            CHECK_EQ(sb.toString(), "3.14");

            StringBuilder sbZero;
            sbZero.append(0.0);
            CHECK_EQ(sbZero.toString(), "0");

            StringBuilder sbNeg;
            sbNeg.append(-1.5);
            CHECK_EQ(sbNeg.toString(), "-1.5");
        }

        TEST_CASE("mixed append round-trips like a JSON-ish fragment")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "{\"a\":" })
                .append(std::int64_t{ 1 })
                .append(std::string_view{ ",\"b\":" })
                .append(2.5)
                .append(std::string_view{ "}" });

            CHECK_EQ(sb.toString(), "{\"a\":1,\"b\":2.5}");
        }

        TEST_CASE("operator+= appends string_view")
        {
            StringBuilder sb;
            sb += std::string_view{ "hello" };
            sb += std::string_view{ ", world" };
            CHECK_EQ(sb.toString(), "hello, world");
        }

        TEST_CASE("operator+= appends char")
        {
            StringBuilder sb;
            sb += 'a';
            sb += 'b';
            sb += 'c';
            CHECK_EQ(sb.toString(), "abc");
        }

        TEST_CASE("operator+= is chainable and equivalent to append")
        {
            StringBuilder sb;
            (sb += std::string_view{ "{\"a\":" }) += '1';
            CHECK_EQ(sb.toString(), "{\"a\":1");
        }

        TEST_CASE("appendRepeated writes count copies of the character")
        {
            StringBuilder sb;
            sb.appendRepeated(' ', 4);
            CHECK_EQ(sb.toString(), "    ");
            CHECK_EQ(sb.size(), 4u);
        }

        TEST_CASE("appendRepeated with count zero is a no-op")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "x" });
            sb.appendRepeated('y', 0);
            CHECK_EQ(sb.toString(), "x");
        }

        TEST_CASE("appendRepeated transitions from stack to heap")
        {
            StringBuilder sb;
            sb.appendRepeated('z', 300);
            CHECK_EQ(sb.size(), 300u);
            CHECK_GT(sb.capacity(), 256u);
            CHECK_EQ(sb.toString(), std::string(300, 'z'));
        }

        TEST_CASE("clear resets size but keeps capacity")
        {
            StringBuilder sb;
            const std::string chunk(300, 'y');
            sb.append(chunk);
            const size_t capacityAfterGrowth = sb.capacity();

            sb.clear();
            CHECK(sb.isEmpty());
            CHECK_EQ(sb.capacity(), capacityAfterGrowth);

            sb.append(std::string_view{ "reused" });
            CHECK_EQ(sb.toString(), "reused");
        }

        TEST_CASE("reserve grows capacity without changing content")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "abc" });
            sb.reserve(1000);

            CHECK_GE(sb.capacity(), 1000u);
            CHECK_EQ(sb.toString(), "abc");
        }

        TEST_CASE("copy constructor - stack buffer")
        {
            StringBuilder original;
            original.append(std::string_view{ "small" });

            StringBuilder copy{ original };
            CHECK_EQ(copy.toString(), "small");

            copy.append(std::string_view{ "!" });
            CHECK_EQ(copy.toString(), "small!");
            CHECK_EQ(original.toString(), "small");
        }

        TEST_CASE("copy constructor - heap buffer")
        {
            StringBuilder original;
            const std::string chunk(300, 'z');
            original.append(chunk);

            StringBuilder copy{ original };
            CHECK_EQ(copy.toString(), chunk);

            copy.append(std::string_view{ "!" });
            CHECK_EQ(original.toString(), chunk);
            CHECK_EQ(copy.size(), chunk.size() + 1);
        }

        TEST_CASE("move constructor - stack buffer")
        {
            StringBuilder original;
            original.append(std::string_view{ "small" });

            StringBuilder moved{ std::move(original) };
            CHECK_EQ(moved.toString(), "small");
            CHECK(original.isEmpty());

            original.append(std::string_view{ "reused" });
            CHECK_EQ(original.toString(), "reused");
        }

        TEST_CASE("move constructor - heap buffer")
        {
            StringBuilder original;
            const std::string chunk(300, 'w');
            original.append(chunk);

            StringBuilder moved{ std::move(original) };
            CHECK_EQ(moved.toString(), chunk);
            CHECK(original.isEmpty());
        }

        TEST_CASE("copy assignment - stack to stack")
        {
            StringBuilder a;
            a.append(std::string_view{ "aaa" });
            StringBuilder b;
            b.append(std::string_view{ "bb" });

            b = a;
            CHECK_EQ(b.toString(), "aaa");
            CHECK_EQ(a.toString(), "aaa");
        }

        TEST_CASE("copy assignment - heap to stack and back")
        {
            StringBuilder a;
            a.append(std::string(300, 'a'));
            StringBuilder b;
            b.append(std::string_view{ "small" });

            b = a;
            CHECK_EQ(b.toString(), std::string(300, 'a'));

            StringBuilder c;
            c.append(std::string_view{ "tiny" });
            b = c;
            CHECK_EQ(b.toString(), "tiny");
        }

        TEST_CASE("move assignment")
        {
            StringBuilder a;
            a.append(std::string(300, 'q'));
            StringBuilder b;
            b.append(std::string_view{ "x" });

            b = std::move(a);
            CHECK_EQ(b.toString(), std::string(300, 'q'));
            CHECK(a.isEmpty());
        }

        TEST_CASE("self-assignment is a no-op")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "self" });

            static_cast<StringBuilder&>(sb) = sb;
            CHECK_EQ(sb.toString(), "self");
        }

        TEST_CASE("view and data are consistent with toString")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "consistent" });

            CHECK_EQ(sb.view(), "consistent");
            CHECK_EQ(std::string_view(sb.data(), sb.size()), "consistent");
            CHECK_EQ(sb.toString(), std::string{ sb.view() });
        }

        TEST_CASE("operator<< streams current content")
        {
            StringBuilder sb;
            sb.append(std::string_view{ "streamed" });

            std::ostringstream oss;
            oss << sb;
            CHECK_EQ(oss.str(), "streamed");
        }
    }
} // namespace dnv::vista::sdk::tests
