#include <doctest/doctest.h>

#include <dnv/vista/sdk/containers/StackVector.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

namespace dnv::vista::sdk::tests
{
    using SVec = StackVector<std::string_view, 16>;

    namespace
    {
        struct Tracked
        {
            static inline int s_liveCount = 0;
            static inline int s_dtorCalls = 0;

            std::unique_ptr<std::string> value;

            explicit Tracked(std::string v = "")
                : value{ std::make_unique<std::string>(std::move(v)) }
            {
                ++s_liveCount;
            }

            Tracked(const Tracked& other)
                : value{ std::make_unique<std::string>(*other.value) }
            {
                ++s_liveCount;
            }

            Tracked(Tracked&& other) noexcept
                : value{ std::move(other.value) }
            {
                ++s_liveCount;
            }

            Tracked& operator=(const Tracked& other)
            {
                if (this != &other)
                {
                    value = std::make_unique<std::string>(*other.value);
                }
                return *this;
            }

            Tracked& operator=(Tracked&& other) noexcept
            {
                if (this != &other)
                {
                    value = std::move(other.value);
                }
                return *this;
            }

            ~Tracked()
            {
                ++s_dtorCalls;
                --s_liveCount;
            }

            bool operator==(const Tracked& other) const { return value && other.value && *value == *other.value; }

            static void resetCounters()
            {
                s_liveCount = 0;
                s_dtorCalls = 0;
            }
        };

        static_assert(!std::is_trivially_copyable_v<Tracked>);
        static_assert(!std::is_trivially_destructible_v<Tracked>);
    } // namespace

    TEST_SUITE("StackVector")
    {
        TEST_CASE("empty construction")
        {
            SVec v;
            CHECK(v.isEmpty());
            CHECK_EQ(v.size(), 0u);
        }

        TEST_CASE("initializer_list construction - within stack capacity")
        {
            SVec v{ "a", "b", "c" };
            CHECK_FALSE(v.isEmpty());
            CHECK_EQ(v.size(), 3u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[1], "b");
            CHECK_EQ(v[2], "c");
        }

        TEST_CASE("initializer_list construction - exceeds stack capacity")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            CHECK_EQ(v.size(), 17u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[16], "q");
        }

        TEST_CASE("push_back - stays on stack")
        {
            SVec v;
            v.push_back("alpha");
            v.push_back("beta");
            CHECK_EQ(v.size(), 2u);
            CHECK_EQ(v[0], "alpha");
            CHECK_EQ(v[1], "beta");
        }

        TEST_CASE("push_back - exact stack capacity")
        {
            SVec v;
            for (int i = 0; i < 16; ++i)
            {
                v.push_back("x");
            }
            CHECK_EQ(v.size(), 16u);
            for (size_t i = 0; i < v.size(); ++i)
            {
                CHECK_EQ(v[i], "x");
            }
        }

        TEST_CASE("push_back - transitions to heap on overflow")
        {
            SVec v;
            for (int i = 0; i < 18; ++i)
            {
                v.push_back("x");
            }
            CHECK_EQ(v.size(), 18u);
            for (size_t i = 0; i < v.size(); ++i)
            {
                CHECK_EQ(v[i], "x");
            }
        }

        TEST_CASE("push_back - all elements preserved after transition")
        {
            SVec v;
            // fill stack exactly
            for (int i = 0; i < 16; ++i)
            {
                v.push_back("stack");
            }
            // trigger transition
            v.push_back("heap1");
            v.push_back("heap2");

            CHECK_EQ(v.size(), 18u);
            for (size_t i = 0; i < 16; ++i)
            {
                CHECK_EQ(v[i], "stack");
            }
            CHECK_EQ(v[16], "heap1");
            CHECK_EQ(v[17], "heap2");
        }

        TEST_CASE("operator[] - read and write")
        {
            SVec v{ "a", "b", "c" };
            v[1] = "z";
            CHECK_EQ(v[1], "z");
        }

        TEST_CASE("iteration - range-for on stack")
        {
            SVec v{ "a", "b", "c" };
            std::string joined;
            for (auto x : v)
            {
                joined += x;
            }
            CHECK_EQ(joined, "abc");
        }

        TEST_CASE("iteration - range-for on heap")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            size_t count = 0;
            for ([[maybe_unused]] auto x : v)
            {
                ++count;
            }
            CHECK_EQ(count, 17u);
        }

        TEST_CASE("iteration - empty is a no-op")
        {
            SVec v;
            int count = 0;
            for ([[maybe_unused]] auto x : v)
            {
                ++count;
            }
            CHECK_EQ(count, 0);
        }

        TEST_CASE("std::is_sorted - compatible with STL algorithms")
        {
            SVec v;
            v.push_back("alpha");
            v.push_back("beta");
            v.push_back("gamma");
            CHECK(std::is_sorted(v.begin(), v.end()));

            SVec w;
            w.push_back("gamma");
            w.push_back("alpha");
            CHECK_FALSE(std::is_sorted(w.begin(), w.end()));
        }

        TEST_CASE("std::find - compatible with STL algorithms")
        {
            SVec v;
            v.push_back("foo");
            v.push_back("bar");
            v.push_back("baz");

            auto it = std::find(v.begin(), v.end(), std::string_view{ "bar" });
            REQUIRE(it != v.end());
            CHECK_EQ(*it, "bar");

            it = std::find(v.begin(), v.end(), std::string_view{ "missing" });
            CHECK(it == v.end());
        }

        TEST_CASE("reserve - within stack capacity is a no-op")
        {
            SVec v{ "a", "b" };
            v.reserve(3);
            CHECK_EQ(v.size(), 2u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[1], "b");
        }

        TEST_CASE("reserve - forces transition to heap")
        {
            SVec v{ "a", "b", "c" };
            v.reserve(20);
            CHECK_EQ(v.size(), 3u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[2], "c");
            // can push_back past stack capacity without issue
            for (int i = 0; i < 17; ++i)
            {
                v.push_back("x");
            }
            CHECK_EQ(v.size(), 20u);
        }

        TEST_CASE("copy construction - stack storage")
        {
            SVec v1{ "a", "b", "c" };
            SVec v2 = v1;
            CHECK_EQ(v2.size(), 3u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[2], "c");
            // original unchanged
            CHECK_EQ(v1[0], "a");
        }

        TEST_CASE("copy construction - heap storage")
        {
            SVec v1{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            SVec v2 = v1;
            CHECK_EQ(v2.size(), 17u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[16], "q");
            // original unchanged
            CHECK_EQ(v1.size(), 17u);
            CHECK_EQ(v1[0], "a");
        }

        TEST_CASE("move construction - stack storage")
        {
            SVec v1{ "a", "b", "c" };
            SVec v2 = std::move(v1);
            CHECK_EQ(v2.size(), 3u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[2], "c");
        }

        TEST_CASE("move construction - heap storage")
        {
            SVec v1{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            SVec v2 = std::move(v1);
            CHECK_EQ(v2.size(), 17u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[16], "q");
        }

        TEST_CASE("copy assignment - stack to stack")
        {
            SVec v1{ "a", "b", "c" };
            SVec v2{ "x", "y" };
            v2 = v1;
            CHECK_EQ(v2.size(), 3u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v1[0], "a");
        }

        TEST_CASE("copy assignment - heap to stack")
        {
            SVec v1{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            SVec v2{ "x", "y" };
            v2 = v1;
            CHECK_EQ(v2.size(), 17u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[16], "q");
            CHECK_EQ(v1.size(), 17u);
        }

        TEST_CASE("copy assignment - stack to heap")
        {
            SVec v1{ "a", "b" };
            SVec v2{ "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x" };
            v2 = v1;
            CHECK_EQ(v2.size(), 2u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[1], "b");
        }

        TEST_CASE("copy assignment - self assignment")
        {
            SVec v{ "a", "b", "c" };
            SVec& ref = v;
            v = ref;
            CHECK_EQ(v.size(), 3u);
            CHECK_EQ(v[0], "a");
        }

        TEST_CASE("move assignment - stack to stack")
        {
            SVec v1{ "a", "b", "c" };
            SVec v2{ "x", "y" };
            v2 = std::move(v1);
            CHECK_EQ(v2.size(), 3u);
            CHECK_EQ(v2[0], "a");
        }

        TEST_CASE("move assignment - heap to stack")
        {
            SVec v1{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            SVec v2{ "x" };
            v2 = std::move(v1);
            CHECK_EQ(v2.size(), 17u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[16], "q");
        }

        TEST_CASE("move assignment - stack to heap")
        {
            SVec v1{ "a", "b" };
            SVec v2{ "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x", "x" };
            v2 = std::move(v1);
            CHECK_EQ(v2.size(), 2u);
            CHECK_EQ(v2[0], "a");
            CHECK_EQ(v2[1], "b");
        }

        TEST_CASE("resize - truncate heap storage stays on heap")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            v.resize(3);
            CHECK_EQ(v.size(), 3u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[2], "c");
            // can still push_back on heap
            v.push_back("z");
            CHECK_EQ(v.size(), 4u);
            CHECK_EQ(v[3], "z");
        }

        TEST_CASE("back - stack storage")
        {
            SVec v{ "a", "b", "c" };
            CHECK_EQ(v.back(), "c");
            v.back() = "z";
            CHECK_EQ(v.back(), "z");
        }

        TEST_CASE("back - heap storage")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            CHECK_EQ(v.back(), "q");
        }

        TEST_CASE("pop_back - reduces size")
        {
            SVec v{ "a", "b", "c" };
            v.pop_back();
            CHECK_EQ(v.size(), 2u);
            CHECK_EQ(v.back(), "b");
        }

        TEST_CASE("pop_back - heap storage")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            v.pop_back();
            CHECK_EQ(v.size(), 16u);
            CHECK_EQ(v.back(), "p");
        }

        TEST_CASE("clear - stack storage")
        {
            SVec v{ "a", "b", "c" };
            v.clear();
            CHECK(v.isEmpty());
            CHECK_EQ(v.size(), 0u);
            v.push_back("z");
            CHECK_EQ(v.size(), 1u);
            CHECK_EQ(v[0], "z");
        }

        TEST_CASE("clear - heap storage stays on heap and retains capacity")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            v.clear();
            CHECK(v.isEmpty());
            // push_back should not reallocate: heap capacity is retained
            for (int i = 0; i < 18; ++i)
            {
                v.push_back("x");
            }
            CHECK_EQ(v.size(), 18u);
            for (size_t i = 0; i < v.size(); ++i)
            {
                CHECK_EQ(v[i], "x");
            }
        }

        TEST_CASE("resize - truncate")
        {
            SVec v{ "a", "b", "c" };
            v.resize(2);
            CHECK_EQ(v.size(), 2u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[1], "b");
        }

        TEST_CASE("resize - extend within stack")
        {
            SVec v{ "a", "b" };
            v.resize(4);
            CHECK_EQ(v.size(), 4u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[1], "b");
            CHECK_EQ(v[2], "");
            CHECK_EQ(v[3], "");
        }

        TEST_CASE("resize - extend beyond stack capacity")
        {
            SVec v{ "a", "b" };
            v.resize(18);
            CHECK_EQ(v.size(), 18u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[1], "b");
            CHECK_EQ(v[2], "");
            CHECK_EQ(v[17], "");
        }

        TEST_CASE("resize - truncate heap storage")
        {
            SVec v{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q" };
            v.resize(2);
            CHECK_EQ(v.size(), 2u);
            CHECK_EQ(v[0], "a");
            CHECK_EQ(v[1], "b");
        }

        TEST_CASE("string_view elements - typical validatePosition use case")
        {
            // Simulates splitting "center-fore-2" on '-'
            SVec parts;
            std::string_view position = "center-fore-2";
            for (std::string_view remaining = position; !remaining.empty();)
            {
                auto sep = remaining.find('-');
                parts.push_back(remaining.substr(0, sep));
                remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);
            }

            CHECK_EQ(parts.size(), 3u);
            CHECK_EQ(parts[0], "center");
            CHECK_EQ(parts[1], "fore");
            CHECK_EQ(parts[2], "2");
        }

        TEST_CASE("pop_back - down to empty then push again stays consistent")
        {
            SVec v{ "a", "b" };
            v.pop_back();
            v.pop_back();
            CHECK(v.isEmpty());
            CHECK_EQ(v.size(), 0u);

            v.push_back("z");
            CHECK_EQ(v.size(), 1u);
            CHECK_EQ(v[0], "z");
            CHECK_EQ(v.back(), "z");
        }

        TEST_CASE("pop_back - down to empty on heap storage then push again stays consistent")
        {
            SVec v{
                "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q"
            }; // forces heap (N=16)
            while (!v.isEmpty())
            {
                v.pop_back();
            }
            CHECK(v.isEmpty());

            v.push_back("z");
            CHECK_EQ(v.size(), 1u);
            CHECK_EQ(v[0], "z");
        }

        TEST_CASE("size() - 1 underflow hazard: caller must guard against empty container")
        {
            SVec v;
            REQUIRE(v.isEmpty());

            size_t iterations = 0;
            for (size_t i = 0; i < v.size() - 1 && iterations < 3; ++i)
            {
                ++iterations; // would run effectively forever without the extra guard
            }
            CHECK_EQ(iterations, 3u); // proves the underflow occurred (loop did not exit at i=0)
        }
    }

    TEST_SUITE("StackVector - non-trivial element type (Tracked)")
    {
        using TVec = StackVector<Tracked, 4>;

        TEST_CASE("construction and destruction on stack - live count balances")
        {
            Tracked::resetCounters();
            {
                TVec v;
                v.emplace_back("a");
                v.emplace_back("b");
                v.emplace_back("c");
                CHECK_EQ(v.size(), 3u);
                CHECK_EQ(Tracked::s_liveCount, 3);
                CHECK_EQ(*v[0].value, "a");
                CHECK_EQ(*v[2].value, "c");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("transition to heap - all stack elements survive the move, none leaked or double-freed")
        {
            Tracked::resetCounters();
            {
                TVec v; // capacity N=4
                v.emplace_back("a");
                v.emplace_back("b");
                v.emplace_back("c");
                v.emplace_back("d");
                CHECK_EQ(Tracked::s_liveCount, 4);

                v.emplace_back("e");
                CHECK_EQ(v.size(), 5u);
                CHECK_EQ(Tracked::s_liveCount, 5);
                CHECK_EQ(*v[0].value, "a");
                CHECK_EQ(*v[1].value, "b");
                CHECK_EQ(*v[2].value, "c");
                CHECK_EQ(*v[3].value, "d");
                CHECK_EQ(*v[4].value, "e");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("copy construction - stack-resident source, deep copies not aliased")
        {
            Tracked::resetCounters();
            {
                TVec original;
                original.emplace_back("x");
                original.emplace_back("y");

                TVec copy{ original };
                CHECK_EQ(Tracked::s_liveCount, 4);

                *copy[0].value = "mutated";
                CHECK_EQ(*original[0].value, "x");
                CHECK_EQ(*copy[0].value, "mutated");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("copy construction - heap-resident source")
        {
            Tracked::resetCounters();
            {
                TVec original;
                for (int i = 0; i < 6; ++i)
                {
                    original.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 6);

                TVec copy{ original };
                CHECK_EQ(Tracked::s_liveCount, 12);
                CHECK_EQ(copy.size(), 6u);
                for (int i = 0; i < 6; ++i)
                {
                    CHECK_EQ(*copy[static_cast<size_t>(i)].value, std::to_string(i));
                }
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("move construction - stack-resident source leaves moved-from empty and destructible")
        {
            Tracked::resetCounters();
            {
                TVec original;
                original.emplace_back("p");
                original.emplace_back("q");
                REQUIRE_EQ(Tracked::s_liveCount, 2);

                TVec moved{ std::move(original) };
                CHECK_EQ(moved.size(), 2u);
                CHECK_EQ(*moved[0].value, "p");
                CHECK(original.isEmpty());
                CHECK_EQ(Tracked::s_liveCount, 2);
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("move construction - heap-resident source")
        {
            Tracked::resetCounters();
            {
                TVec original;
                for (int i = 0; i < 6; ++i)
                {
                    original.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 6);

                TVec moved{ std::move(original) };
                CHECK_EQ(moved.size(), 6u);
                CHECK(original.isEmpty());
                CHECK_EQ(Tracked::s_liveCount, 6);
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("copy assignment - state-kind change: heap source assigned onto stack-resident destination")
        {
            Tracked::resetCounters();
            {
                TVec dest;
                dest.emplace_back("only");
                REQUIRE_EQ(Tracked::s_liveCount, 1);

                TVec source;
                for (int i = 0; i < 6; ++i)
                {
                    source.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 7);

                dest = source;
                CHECK_EQ(dest.size(), 6u);
                CHECK_EQ(Tracked::s_liveCount, 12);
                CHECK_EQ(*dest[0].value, "0");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("copy assignment - state-kind change: stack source assigned onto heap-resident destination")
        {
            Tracked::resetCounters();
            {
                TVec dest;
                for (int i = 0; i < 6; ++i)
                {
                    dest.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 6);

                TVec source;
                source.emplace_back("only");
                REQUIRE_EQ(Tracked::s_liveCount, 7);

                dest = source;
                CHECK_EQ(dest.size(), 1u);
                CHECK_EQ(Tracked::s_liveCount, 2);
                CHECK_EQ(*dest[0].value, "only");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("move assignment - state-kind change: heap source assigned onto stack-resident destination")
        {
            Tracked::resetCounters();
            {
                TVec dest;
                dest.emplace_back("only");
                REQUIRE_EQ(Tracked::s_liveCount, 1);

                TVec source;
                for (int i = 0; i < 6; ++i)
                {
                    source.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 7);

                dest = std::move(source);
                CHECK_EQ(dest.size(), 6u);
                CHECK(source.isEmpty());
                CHECK_EQ(Tracked::s_liveCount, 6);
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("self-assignment - copy and move are both no-ops that don't corrupt state (stack-resident)")
        {
            Tracked::resetCounters();
            {
                TVec v;
                v.emplace_back("a");
                v.emplace_back("b");
                v.emplace_back("c");
                REQUIRE_EQ(Tracked::s_liveCount, 3);

                TVec* self = &v;
                v = *self;
                CHECK_EQ(v.size(), 3u);
                CHECK_EQ(Tracked::s_liveCount, 3);
                CHECK_EQ(*v[0].value, "a");

                v = std::move(*self);
                CHECK_EQ(v.size(), 3u);
                CHECK_EQ(Tracked::s_liveCount, 3);
                CHECK_EQ(*v[2].value, "c");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("self-assignment - copy and move are both no-ops that don't corrupt state (heap-resident)")
        {
            Tracked::resetCounters();
            {
                TVec v;
                for (int i = 0; i < 6; ++i)
                {
                    v.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 6);

                TVec* self = &v;
                v = *self;
                CHECK_EQ(v.size(), 6u);
                CHECK_EQ(Tracked::s_liveCount, 6);
                CHECK_EQ(*v[0].value, "0");
                CHECK_EQ(*v[5].value, "5");

                v = std::move(*self);
                CHECK_EQ(v.size(), 6u);
                CHECK_EQ(Tracked::s_liveCount, 6);
                CHECK_EQ(*v[0].value, "0");
                CHECK_EQ(*v[5].value, "5");
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("pop_back and clear correctly destroy non-trivial elements")
        {
            Tracked::resetCounters();
            {
                TVec v;
                v.emplace_back("a");
                v.emplace_back("b");
                v.emplace_back("c");
                REQUIRE_EQ(Tracked::s_liveCount, 3);

                v.pop_back();
                CHECK_EQ(v.size(), 2u);
                CHECK_EQ(Tracked::s_liveCount, 2);
                CHECK_EQ(Tracked::s_dtorCalls, 1);

                v.clear();
                CHECK(v.isEmpty());
                CHECK_EQ(Tracked::s_liveCount, 0);
                CHECK_EQ(Tracked::s_dtorCalls, 3);
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }

        TEST_CASE("resize down destroys the truncated tail exactly once each")
        {
            Tracked::resetCounters();
            {
                TVec v;
                for (int i = 0; i < 6; ++i)
                {
                    v.emplace_back(std::to_string(i));
                }
                REQUIRE_EQ(Tracked::s_liveCount, 6);

                Tracked::s_dtorCalls = 0;

                v.resize(2);
                CHECK_EQ(v.size(), 2u);
                CHECK_EQ(Tracked::s_liveCount, 2);
                CHECK_EQ(Tracked::s_dtorCalls, 4);
            }
            CHECK_EQ(Tracked::s_liveCount, 0);
        }
    }
} // namespace dnv::vista::sdk::tests
