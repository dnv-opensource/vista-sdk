#include <doctest/doctest.h>

#include <dnv/vista/sdk/detail/types/decimal/Constants.h>
#include <dnv/vista/sdk/types/decimal/Decimal.h>

#include <SDK/types/decimal/Int128.h>

#include <format>
#include <limits>
#include <sstream>

namespace dnv::vista::sdk::tests
{
    using internal::Int128;
    using internal::constants::INT128_MAX_POSITIVE_HIGH;
    using internal::constants::INT128_MAX_POSITIVE_LOW;
    using internal::constants::INT128_MIN_NEGATIVE_HIGH;

    TEST_SUITE("Int128")
    {
        TEST_CASE("construction - default")
        {
            Int128 zero;
            CHECK(zero == 0);
            CHECK_FALSE(zero < 0);
            CHECK_EQ(0ULL, zero.toLow());
            CHECK_EQ(0ULL, zero.toHigh());
        }

        TEST_CASE("construction - from uint64")
        {
            const std::uint64_t value{ 0x123456789ABCDEFULL };
            Int128 num{ value };

            CHECK_FALSE(num == 0);
            CHECK_FALSE(num < 0);
            CHECK_EQ(value, num.toLow());
            CHECK_EQ(0ULL, num.toHigh());
        }

        TEST_CASE("construction - from int64 positive")
        {
            const std::int64_t value{ 0x123456789ABCDEFLL };
            Int128 num{ value };

            CHECK_FALSE(num == 0);
            CHECK_FALSE(num < 0);
            CHECK_EQ(static_cast<std::uint64_t>(value), num.toLow());
            CHECK_EQ(0ULL, num.toHigh());
        }

        TEST_CASE("construction - from int64 negative")
        {
            const std::int64_t value{ -0x123456789ABCDEFLL };
            Int128 num{ value };

            CHECK_FALSE(num == 0);
            CHECK(num < 0);
            CHECK_EQ(static_cast<std::uint64_t>(value), num.toLow());
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, num.toHigh()); // sign extension
        }

        TEST_CASE("construction - from uint32")
        {
            const std::uint32_t value{ 0x12345678U };
            Int128 num{ value };

            CHECK_FALSE(num == 0);
            CHECK_FALSE(num < 0);
            CHECK_EQ(static_cast<std::uint64_t>(value), num.toLow());
            CHECK_EQ(0ULL, num.toHigh());
        }

        TEST_CASE("construction - from int32 positive")
        {
            const int value{ 0x12345678 };
            Int128 num{ value };

            CHECK_FALSE(num == 0);
            CHECK_FALSE(num < 0);
            CHECK_EQ(static_cast<std::uint64_t>(value), num.toLow());
            CHECK_EQ(0ULL, num.toHigh());
        }

        TEST_CASE("construction - from int32 negative")
        {
            const int value{ -0x12345678 };
            Int128 num{ value };

            CHECK_FALSE(num == 0);
            CHECK(num < 0);
            CHECK_EQ(static_cast<std::uint64_t>(value), num.toLow());
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, num.toHigh()); // sign extension
        }

        TEST_CASE("construction - from low/high pair")
        {
            const std::uint64_t low{ 0x123456789ABCDEFULL };
            const std::uint64_t high{ 0xFEDCBA9876543210ULL };
            Int128 num{ low, high };

            CHECK_FALSE(num == 0);
            CHECK(num < 0);
            CHECK_EQ(low, num.toLow());
            CHECK_EQ(high, num.toHigh());
        }

        TEST_CASE("construction - from double")
        {
            CHECK_EQ(Int128{ 42.7 }.toString(), "42");
            CHECK_FALSE(Int128{ 42.7 } < 0);

            CHECK_EQ(Int128{ -42.7 }.toString(), "-42");
            CHECK(Int128{ -42.7 } < 0);

            CHECK_EQ(Int128{ 123456.0 }.toString(), "123456");

            CHECK(Int128{ 0.0 } == 0);
            CHECK(Int128{ -0.0 } == 0);
            CHECK(Int128{ 0.9999 } == 0);
            CHECK(Int128{ -0.9999 } == 0);

            CHECK_EQ(Int128{ 1.0 }.toString(), "1");

            CHECK(Int128{ std::numeric_limits<double>::quiet_NaN() } == 0);

            // Consistency with C++ cast
            double testValue = 12.6;
            CHECK_EQ(Int128{ testValue }.toString(), std::to_string(static_cast<int>(testValue)));
        }

        TEST_CASE("construction - from Decimal")
        {
            CHECK_EQ(42ULL, Int128{ Decimal{ 42 } }.toLow());
            CHECK_FALSE(Int128{ Decimal{ 42 } } < 0);

            CHECK(Int128{ Decimal{ -123 } } < 0);
            CHECK_EQ(Int128{ Decimal{ -123 } }.toString(), "-123");

            CHECK(Int128{ Decimal{ 0 } } == 0);

            Int128 fromLarge{ Decimal{ "12345678901234567890" } };
            CHECK_EQ(fromLarge.toString(), "12345678901234567890");

            // Fractional part is truncated toward zero
            CHECK_EQ(Int128{ Decimal{ "42.5" } }.toString(), "42");
            CHECK_EQ(Int128{ Decimal{ "-456.789" } }.toString(), "-456");

            // Roundtrip
            Decimal original{ "987654321098765432109876" };
            Decimal backToDecimal{ Int128{ original }.toString() };
            CHECK(original == backToDecimal);
        }

        TEST_CASE("arithmetic - addition")
        {
            Int128 result{ Int128{ 100 } + Int128{ 200 } };
            CHECK_EQ(300ULL, result.toLow());
            CHECK_EQ(0ULL, result.toHigh());
        }

        TEST_CASE("arithmetic - addition with carry")
        {
            Int128 a{ INT128_MAX_POSITIVE_LOW, 0 };
            Int128 result{ a + Int128{ 1 } };
            CHECK_EQ(0ULL, result.toLow());
            CHECK_EQ(1ULL, result.toHigh());
        }

        TEST_CASE("arithmetic - subtraction")
        {
            Int128 result{ Int128{ 300 } - Int128{ 100 } };
            CHECK_EQ(200ULL, result.toLow());
            CHECK_EQ(0ULL, result.toHigh());
        }

        TEST_CASE("arithmetic - subtraction with borrow")
        {
            Int128 a{ 0ULL, 1ULL };
            Int128 result{ a - Int128{ 1 } };
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, result.toLow());
            CHECK_EQ(0ULL, result.toHigh());
        }

        TEST_CASE("arithmetic - multiplication")
        {
            Int128 result{ Int128{ 123 } * Int128{ 456 } };
            CHECK_EQ(123ULL * 456ULL, result.toLow());
            CHECK_EQ(0ULL, result.toHigh());
        }

        TEST_CASE("arithmetic - multiplication large")
        {
            const std::uint64_t a_val{ 0x123456789ABCDEFULL };
            const std::uint64_t b_val{ std::uint64_t{ 0x100000000ULL } };

            Int128 result{ Int128{ a_val } * Int128{ b_val } };
            CHECK_EQ((a_val << 32) & INT128_MAX_POSITIVE_LOW, result.toLow());
            CHECK_EQ(a_val >> 32, result.toHigh());
        }

        TEST_CASE("arithmetic - division simple")
        {
            CHECK_EQ(3ULL, (Int128{ 456 } / Int128{ 123 }).toLow());
            CHECK_EQ(
                333333333333ULL,
                (Int128{ std::uint64_t{ 1000000000000ULL } } / Int128{ std::uint64_t{ 3ULL } }).toLow());
        }

        TEST_CASE("arithmetic - division 128-bit dividend")
        {
            Int128 bigDividend{ "20000000000000000000000000000000000000" };
            Int128 quotient{ bigDividend / Int128{ std::uint64_t{ 1416666666666666665ULL } } };
            CHECK_EQ(Int128{ "14117647058823529428" }, quotient);

            Int128 big1{ "1000000000000000000000" };
            CHECK_EQ(Int128{ "142857142857142857142" }, big1 / Int128{ std::uint64_t{ 7ULL } });
        }

        TEST_CASE("arithmetic - division by zero throws")
        {
            CHECK_THROWS_AS(Int128{ 123 } / Int128{}, std::overflow_error);
        }

        TEST_CASE("arithmetic - modulo")
        {
            CHECK_EQ(87ULL, (Int128{ 456 } % Int128{ 123 }).toLow());
        }

        TEST_CASE("arithmetic - modulo by zero throws")
        {
            CHECK_THROWS_AS(Int128{ 123 } % Int128{}, std::overflow_error);
        }

        TEST_CASE("arithmetic - unary minus")
        {
            Int128 positive{ 123 };
            Int128 negative{ -positive };

            CHECK(negative < 0);
            CHECK_FALSE(negative == 0);
            CHECK_EQ(static_cast<std::uint64_t>(-123), negative.toLow());
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, negative.toHigh());
        }

        TEST_CASE("arithmetic - unary minus zero")
        {
            Int128 negZero{ -Int128{} };
            CHECK(negZero == 0);
            CHECK_FALSE(negZero < 0);
        }

        TEST_CASE("arithmetic - double negation")
        {
            Int128 original{ 777 };
            CHECK_EQ(original, -(-original));
        }

        TEST_CASE("arithmetic - compound assignment")
        {
            Int128 a{ 100 };
            a += Int128{ 50 };
            CHECK_EQ(a, Int128{ 150 });

            a += Int128{ -30 };
            CHECK_EQ(a, Int128{ 120 });

            Int128 b{ 200 };
            b -= Int128{ 75 };
            CHECK_EQ(b, Int128{ 125 });

            Int128 c{ 12 };
            c *= Int128{ 5 };
            CHECK_EQ(c, Int128{ 60 });

            c *= Int128{ -2 };
            CHECK_EQ(c, Int128{ -120 });

            Int128 d{ 100 };
            d /= Int128{ 5 };
            CHECK_EQ(d, Int128{ 20 });

            CHECK_THROWS_AS(d /= Int128{}, std::overflow_error);

            Int128 f{ 100 };
            f %= Int128{ 7 };
            CHECK_EQ(f, Int128{ 2 });

            CHECK_THROWS_AS(f %= Int128{}, std::overflow_error);

            // ((10 + 5) * 2 - 10) / 2 = 10
            Int128 h{ 10 };
            h += Int128{ 5 };
            h *= Int128{ 2 };
            h -= Int128{ 10 };
            h /= Int128{ 2 };
            CHECK_EQ(h, Int128{ 10 });
        }

        TEST_CASE("arithmetic - abs")
        {
            CHECK_EQ(Int128{ 123 }, Int128{ 123 }.abs());
            CHECK_EQ(Int128{ 123 }, Int128{ -123 }.abs());
            CHECK_EQ(Int128{}, Int128{}.abs());
        }

        TEST_CASE("arithmetic - isqrt perfect squares")
        {
            CHECK_EQ(Int128{ 2 }, Int128{ 4 }.isqrt());
            CHECK_EQ(Int128{ 3 }, Int128{ 9 }.isqrt());
            CHECK_EQ(Int128{ 10 }, Int128{ 100 }.isqrt());
            CHECK_EQ(Int128{}, Int128{}.isqrt());
            CHECK_EQ(Int128{ 1 }, Int128{ 1 }.isqrt());
            CHECK_EQ(Int128{ 1000000 }, Int128{ "1000000000000" }.isqrt());
            CHECK_EQ(Int128{ "1000000000" }, Int128{ "1000000000000000000" }.isqrt());
        }

        TEST_CASE("arithmetic - isqrt non-perfect squares (floor)")
        {
            CHECK_EQ(Int128{ 2 }, Int128{ 5 }.isqrt());
            CHECK_EQ(Int128{ 3 }, Int128{ 10 }.isqrt());
            CHECK_EQ(Int128{ 9 }, Int128{ 99 }.isqrt());
            CHECK_EQ(Int128{ 10 }, Int128{ 101 }.isqrt());
        }

        TEST_CASE("arithmetic - isqrt result invariant")
        {
            Int128 d{ 50 };
            Int128 s{ d.isqrt() };
            CHECK(s * s <= d);
            CHECK((s + Int128{ 1 }) * (s + Int128{ 1 }) > d);
        }

        TEST_CASE("arithmetic - isqrt negative throws")
        {
            CHECK_THROWS_AS([[maybe_unused]] auto r = Int128{ -4 }.isqrt(), std::domain_error);
        }

        TEST_CASE("arithmetic - sqrt perfect squares")
        {
            CHECK_EQ(Decimal{ 2 }, sqrt(Int128{ 4 }));
            CHECK_EQ(Decimal{ 3 }, sqrt(Int128{ 9 }));
            CHECK_EQ(Decimal{ 10 }, sqrt(Int128{ 100 }));
            CHECK_EQ(Decimal{ 0 }, sqrt(Int128{}));
            CHECK_EQ(Decimal{ 1 }, sqrt(Int128{ 1 }));
            CHECK_EQ(Decimal{ 100000 }, sqrt(Int128{ "10000000000" }));
        }

        TEST_CASE("arithmetic - sqrt non-perfect squares")
        {
            auto s2 = sqrt(Int128{ 2 });
            CHECK(s2 > Decimal{ "1.414" });
            CHECK(s2 < Decimal{ "1.415" });

            auto s3 = sqrt(Int128{ 3 });
            CHECK(s3 > Decimal{ "1.732" });
            CHECK(s3 < Decimal{ "1.733" });

            auto s50 = sqrt(Int128{ 50 });
            CHECK(s50 > Decimal{ "7.07" });
            CHECK(s50 < Decimal{ "7.08" });

            auto s1000 = sqrt(Int128{ 1000 });
            CHECK(s1000 > Decimal{ "31.62" });
            CHECK(s1000 < Decimal{ "31.63" });
        }

        TEST_CASE("arithmetic - sqrt large perfect squares")
        {
            CHECK_EQ(Decimal{ "10000000000" }, sqrt(Int128{ "100000000000000000000" })); // sqrt(10^20) = 10^10

            Int128 base{ 999999999 };
            CHECK_EQ(Decimal{ 999999999 }, sqrt(base * base));

            Int128 base2{ 123456789 };
            CHECK_EQ(Decimal{ 123456789 }, sqrt(base2 * base2));
        }

        TEST_CASE("arithmetic - sqrt negative throws")
        {
            CHECK_THROWS_AS([[maybe_unused]] auto r = sqrt(Int128{ -4 }), std::domain_error);
        }

        TEST_CASE("comparison - equality")
        {
            CHECK(Int128{ 123 } == Int128{ 123 });
            CHECK_FALSE(Int128{ 123 } == Int128{ 456 });
            CHECK_FALSE(Int128{ 123 } != Int128{ 123 });
            CHECK(Int128{ 123 } != Int128{ 456 });
        }

        TEST_CASE("comparison - ordering")
        {
            CHECK(Int128{ 123 } < Int128{ 456 });
            CHECK_FALSE(Int128{ 456 } < Int128{ 123 });
            CHECK_FALSE(Int128{ 123 } < Int128{ 123 });

            CHECK(Int128{ -456 } < Int128{ 123 });
            CHECK_FALSE(Int128{ 123 } < Int128{ -456 });

            CHECK(Int128{ 123 } <= Int128{ 456 });
            CHECK(Int128{ 123 } <= Int128{ 123 });
            CHECK_FALSE(Int128{ 456 } <= Int128{ 123 });

            CHECK(Int128{ 456 } > Int128{ 123 });
            CHECK_FALSE(Int128{ 123 } > Int128{ 456 });

            CHECK(Int128{ 456 } >= Int128{ 123 });
            CHECK(Int128{ 123 } >= Int128{ 123 });
            CHECK_FALSE(Int128{ 123 } >= Int128{ 456 });
        }

        TEST_CASE("comparison - high word differs")
        {
            Int128 a{ INT128_MAX_POSITIVE_LOW, 0 };
            Int128 b{ 0ULL, 1ULL };
            CHECK(a < b);
            CHECK_FALSE(b < a);
        }

        TEST_CASE("comparison - with int64")
        {
            Int128 i{ 42 };
            CHECK(i == std::int64_t{ 42 });
            CHECK_FALSE(i == std::int64_t{ 100 });
            CHECK(i < std::int64_t{ 100 });
            CHECK_FALSE(i < std::int64_t{ 42 });
            CHECK(Int128{ -123 } < std::int64_t{ 0 });
        }

        TEST_CASE("comparison - with uint64")
        {
            Int128 i{ 42 };
            CHECK(i == std::uint64_t{ 42 });
            CHECK(Int128{ -1 } < std::uint64_t{ 0 });
            CHECK_FALSE(Int128{ -1 } > std::uint64_t{ 0 });
        }

        TEST_CASE("comparison - with int")
        {
            Int128 i{ 42 };
            CHECK(i == 42);
            CHECK_FALSE(i == 100);
            CHECK(i < 100);
            CHECK(i > -1);
        }

        TEST_CASE("comparison - with double")
        {
            CHECK(Int128{ 42 } == 42.0);
            CHECK(Int128{ 42 } < 42.1);
            CHECK(Int128{ 42 } > 41.9);
            CHECK(Int128{ -123 } == -123.0);
            CHECK(Int128{} == 0.0);
            CHECK(Int128{} == -0.0);
        }

        TEST_CASE("comparison - NaN always false")
        {
            double nan = std::numeric_limits<double>::quiet_NaN();
            CHECK_FALSE(Int128{ 123 } == nan);
            CHECK(Int128{ 123 } != nan);
            CHECK_FALSE(Int128{ 123 } < nan);
            CHECK_FALSE(Int128{ 123 } > nan);
        }

        TEST_CASE("comparison - with Decimal")
        {
            CHECK(Int128{ 42 } == Decimal{ 42 });
            CHECK_FALSE(Int128{ 42 } == Decimal{ -42 });
            CHECK(Int128{ 100 } < Decimal{ 200 });
            CHECK(Int128{ 300 } > Decimal{ 200 });
            CHECK(Int128{ 42 } < Decimal{ "42.7" });
            CHECK(Int128{ 43 } > Decimal{ "42.3" });
        }

        TEST_CASE("comparison - state: zero / negative")
        {
            CHECK(Int128{} == 0);
            CHECK_FALSE(Int128{ 1 } == 0);
            CHECK(Int128{ -123 } < 0);
            CHECK_FALSE(Int128{ 123 } < 0);
            CHECK_FALSE(Int128{} < 0);
        }

        TEST_CASE("string - toString")
        {
            CHECK_EQ(Int128{ 123 }.toString(), "123");
            CHECK_EQ(Int128{ -456 }.toString(), "-456");
            CHECK_EQ(Int128{}.toString(), "0");
        }

        TEST_CASE("string - fromString valid")
        {
            Int128 result;
            CHECK(Int128::fromString("12345", result));
            CHECK_EQ(12345ULL, result.toLow());
            CHECK_FALSE(result < 0);

            CHECK(Int128::fromString("-9876543210", result));
            CHECK(result < 0);

            CHECK(Int128::fromString("0", result));
            CHECK(result == 0);

            CHECK(Int128::fromString("+42", result));
            CHECK_EQ(42ULL, result.toLow());

            CHECK(Int128::fromString("123456789012345678901234567890", result));
            CHECK_FALSE(result == 0);

            CHECK(Int128::fromString("170141183460469231731687303715884105727", result));
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, result.toLow());
            CHECK_EQ(INT128_MAX_POSITIVE_HIGH, result.toHigh());
        }

        TEST_CASE("string - fromString invalid")
        {
            Int128 result;
            CHECK_FALSE(Int128::fromString("", result));
            CHECK_FALSE(Int128::fromString("abc", result));
            CHECK_FALSE(Int128::fromString("12.34", result));
            CHECK_FALSE(Int128::fromString("+", result));
            CHECK_FALSE(Int128::fromString("-", result));
            CHECK_FALSE(Int128::fromString("1e10", result));
            CHECK_FALSE(Int128::fromString("170141183460469231731687303715884105728", result));
        }

        TEST_CASE("string - constructor from const char* valid")
        {
            CHECK_EQ(Int128{ "12345" }.toLow(), 12345ULL);
            CHECK_FALSE(Int128{ "12345" } < 0);
            CHECK(Int128{ "-9876543210" } < 0);
            CHECK(Int128{ "0" } == 0);
            CHECK_EQ(Int128{ "+42" }.toLow(), 42ULL);
        }

        TEST_CASE("string - constructor from const char* invalid throws")
        {
            CHECK_THROWS_AS([[maybe_unused]] Int128 r{ "" }, std::invalid_argument);
            CHECK_THROWS_AS([[maybe_unused]] Int128 r{ "abc" }, std::invalid_argument);
            CHECK_THROWS_AS([[maybe_unused]] Int128 r{ "12.34" }, std::invalid_argument);
        }

        TEST_CASE("numeric_limits")
        {
            auto maxVal = std::numeric_limits<Int128>::max();
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, maxVal.toLow());
            CHECK_EQ(INT128_MAX_POSITIVE_HIGH, maxVal.toHigh());
            CHECK_FALSE(maxVal < 0);

            auto minVal = std::numeric_limits<Int128>::min();
            CHECK_EQ(0ULL, minVal.toLow());
            CHECK_EQ(INT128_MIN_NEGATIVE_HIGH, minVal.toHigh());
            CHECK(minVal < 0);
        }

        TEST_CASE("edge - max positive / min negative")
        {
            Int128 maxPositive{ INT128_MAX_POSITIVE_LOW, INT128_MAX_POSITIVE_HIGH };
            CHECK_FALSE(maxPositive < 0);

            Int128 minNegative{ 0ULL, INT128_MIN_NEGATIVE_HIGH };
            CHECK(minNegative < 0);

            CHECK(maxPositive > minNegative);
        }

        TEST_CASE("edge - addition overflow wraps")
        {
            Int128 maxPositive{ INT128_MAX_POSITIVE_LOW, INT128_MAX_POSITIVE_HIGH };
            Int128 wrapped{ maxPositive + Int128{ 1 } };
            CHECK(wrapped < 0);
            CHECK_EQ(0ULL, wrapped.toLow());
            CHECK_EQ(INT128_MIN_NEGATIVE_HIGH, wrapped.toHigh());
        }

        TEST_CASE("edge - subtraction underflow wraps")
        {
            Int128 minNegative{ 0ULL, INT128_MIN_NEGATIVE_HIGH };
            Int128 wrapped{ minNegative - Int128{ 1 } };
            CHECK_FALSE(wrapped < 0);
            CHECK_EQ(INT128_MAX_POSITIVE_LOW, wrapped.toLow());
            CHECK_EQ(INT128_MAX_POSITIVE_HIGH, wrapped.toHigh());
        }

        TEST_CASE("edge - sign bit boundary")
        {
            Int128 justPositive{ INT128_MAX_POSITIVE_LOW, INT128_MAX_POSITIVE_HIGH };
            Int128 justNegative{ 0ULL, INT128_MIN_NEGATIVE_HIGH };

            CHECK_FALSE(justPositive < 0);
            CHECK(justNegative < 0);
            CHECK_EQ(justPositive + Int128{ 1 }, justNegative);
        }

        TEST_CASE("edge - negation boundary")
        {
            Int128 maxPositive{ INT128_MAX_POSITIVE_LOW, INT128_MAX_POSITIVE_HIGH };
            Int128 minNegative{ 0ULL, INT128_MIN_NEGATIVE_HIGH };

            // -minNegative overflows back to itself (two's complement)
            CHECK_EQ(-minNegative, minNegative);
            // -maxPositive is representable (minNegative + 1)
            CHECK_EQ(-maxPositive, minNegative + Int128{ 1ULL, 0ULL });
        }

        TEST_CASE("edge - abs of min negative wraps around")
        {
            Int128 minNegative{ 0ULL, INT128_MIN_NEGATIVE_HIGH };

            // abs(minNegative) has no positive representable magnitude (2^127 doesn't fit)
            // wraps back to itself, same convention as negation.
            CHECK_EQ(minNegative.abs(), minNegative);
            CHECK_EQ(abs(minNegative), minNegative);
        }

        TEST_CASE("edge - division of min negative by minus one wraps around")
        {
            Int128 minNegative{ 0ULL, INT128_MIN_NEGATIVE_HIGH };
            Int128 minusOne{ ~std::uint64_t{ 0 }, ~std::uint64_t{ 0 } };

            CHECK_EQ(minusOne, Int128{ -1 });

            Int128 result{ minNegative / minusOne };
            CHECK_EQ(result, minNegative);
        }

        TEST_CASE("edge - parse limits")
        {
            Int128 r;
            CHECK(Int128::fromString("170141183460469231731687303715884105727", r));
            CHECK_FALSE(r < 0);

            CHECK(Int128::fromString("-170141183460469231731687303715884105728", r));
            CHECK(r < 0);

            // Beyond 128-bit range
            CHECK_FALSE(Int128::fromString("170141183460469231731687303715884105728", r));
            CHECK_FALSE(Int128::fromString("-170141183460469231731687303715884105729", r));
        }

        TEST_CASE("edge - toBits")
        {
            auto zeroBits = Int128{}.toBits();
            CHECK_EQ(zeroBits.size(), 4);
            CHECK_EQ(zeroBits[0], 0);
            CHECK_EQ(zeroBits[1], 0);
            CHECK_EQ(zeroBits[2], 0);
            CHECK_EQ(zeroBits[3], 0);

            auto bits = Int128{ 123456 }.toBits();
            CHECK_EQ(bits[0], 123456);
            CHECK_EQ(bits[1], 0);
            CHECK_EQ(bits[2], 0);
            CHECK_EQ(bits[3], 0);

            // -1 -> all bits set (sign extension)
            auto negBits = Int128{ -1 }.toBits();
            CHECK_EQ(negBits[0], -1);
            CHECK_EQ(negBits[1], -1);
            CHECK_EQ(negBits[2], -1);
            CHECK_EQ(negBits[3], -1);
        }

        TEST_CASE("edge - isqrt max value")
        {
            Int128 maxValue = std::numeric_limits<Int128>::max();
            Int128 maxSqrt = maxValue.isqrt();

            CHECK(maxSqrt * maxSqrt <= maxValue);
            Int128 remainder = maxValue - maxSqrt * maxSqrt;
            CHECK(remainder < Int128{ 2 } * maxSqrt + Int128{ 1 });
            CHECK_EQ(maxSqrt, Int128{ "13043817825332782212" });
        }
    }
} // namespace dnv::vista::sdk::tests
